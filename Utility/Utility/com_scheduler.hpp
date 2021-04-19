/**
* @file com_scheduler.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __COM_SCHEDULER_HPP__
#define __COM_SCHEDULER_HPP__

#include "com_thread_pool.hpp"
#include "mem_data_factory.hpp"
#include <chrono>
#include <atomic>
#include <map>

//#define SCHEDULER_LOG

#ifdef SCHEDULER_LOG
#define SCHEDULER_DEBUG(fmt,...) Clog::debug(fmt,##__VA_ARGS__);
#else
#define SCHEDULER_DEBUG(fmt,...)
#endif

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace com
{
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class _Clock, class _Duration = typename _Clock::duration>
class scheduler
{
private:
	using time_point = std::chrono::time_point<_Clock, _Duration>;
	using func_t = std::function<void()>;
	struct event_t
	{
		event_t(void) = default;
		~event_t(void) = default;
		std::atomic<std::uint32_t> m_siCode;
		func_t m_func;
	};
public:
	class Event {
	public:
		friend class scheduler;
		~Event(void) = default;
		inline bool cancel(void) { return m_event ? m_event->m_siCode.compare_exchange_strong(m_siCode, 0) : false; }
		operator bool (void){ return m_event ? m_event->m_siCode == m_siCode : false; }
	private:
		Event(std::uint32_t siCode, event_t* ev) :m_siCode(siCode), m_event(ev) {}

		std::uint32_t m_siCode;
		event_t* m_event;
	};
private:
	com::threadpool m_threadpool;
	std::thread m_main_thread;
	std::multimap<time_point, Event> tasks;
	mem::data_factory_ex<event_t, 50> m_event_pool;
	std::mutex mtx;
	std::condition_variable cv;
	std::atomic_size_t m_ntasks = { 0 };
	bool m_running = false;
	std::uint32_t m_siCode = 0;
public:
	scheduler(void) = default;
	~scheduler(void) = default;
public:
	void init(size_t size)
	{
		assert(size != 0);
		std::unique_lock<std::mutex> lock(mtx);
		if (m_running) return;
		m_running = true;
		lock.unlock();
		m_main_thread = std::thread(&scheduler<_Clock,_Duration>::run_task,this);
		m_threadpool.init(size);
	}

	template<class F, class... Args>
	Event attach(time_point tp, F&& f, Args&&... args)
	{
		std::lock_guard<std::mutex> lock(mtx);
		if (!m_running)
			return {0,nullptr};

		if (++m_siCode == 0)
			m_siCode = 1;
		event_t* ev = m_event_pool.malloc();
		ev->m_siCode = m_siCode;
		ev->m_func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
		auto iter = tasks.emplace(std::make_pair(tp, Event{ m_siCode, ev }));

		++m_ntasks;
		cv.notify_one();
		return iter->second;
	}

	// 关闭任务提交 等待剩余任务完成 (调用时请确保pool不会析构)
	bool stop(void)
	{
		std::unique_lock<std::mutex> lock(mtx);
		if (!m_running) return false;
		m_running = false;
		lock.unlock();
		cv.notify_one();
		SCHEDULER_DEBUG("stop:%lld", std::chrono::system_clock::now().time_since_epoch().count())
		m_main_thread.join();
		m_threadpool.safe_stop();
		return true;
	}

	size_t task_size(void) { return m_ntasks; }
private:
	void run_task(void)
	{
		do {
			std::unique_lock<std::mutex> lock(mtx);
			if (tasks.empty())
				cv.wait(lock);

			if (!m_running)
				return;

			auto begin = tasks.begin();
			cv.wait_until(lock, begin->first);

			if (!m_running)
				return;

			time_point now = std::chrono::system_clock::now();
			if (begin->first > now) 
				continue;

			auto iter = begin;
			SCHEDULER_DEBUG("now:%lld", now.time_since_epoch().count())
			do {
				SCHEDULER_DEBUG("evt:%lld", iter->first.time_since_epoch().count())
				if (iter->second.cancel())
					m_threadpool.schedule(std::move(iter->second.m_event->m_func));
				m_event_pool.free(iter->second.m_event);
				--m_ntasks;
			} while (++iter != tasks.end() && iter->first <= now);
			tasks.erase(begin,iter);
		} while (true);
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}// namspace com
////////////////////////////////////////////////////////////////////////////////////////////////////
}// namespace Utility
#endif //__COM_SCHEDULER_HPP__ 