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
class timer
{
	using func_t = std::function<void()>;
	struct event_t
	{
		event_t(void) = default;
		~event_t(void) = default;
		std::atomic<std::uint32_t> m_siCode = 0;
		std::uint32_t m_count = 0;
		func_t m_func;
	};
	template<class,class> friend class scheduler;
public:
	timer(void) = default;
	~timer(void) = default;
	inline bool cancel(void) { return m_event ? m_event->m_siCode.compare_exchange_strong(m_siCode, 0) : false; }
	operator bool(void) { return m_event ? m_event->m_siCode == m_siCode : false; }
private:
	timer(std::uint32_t siCode, event_t * ev) :m_siCode(siCode), m_event(ev) {}

	std::uint32_t m_siCode = 1;
	event_t* m_event = nullptr;
};

template<class _Clock, class _Duration = typename _Clock::duration>
class scheduler
{
	using clock = _Clock;
	using time_point = typename clock::time_point;
	using event_t = timer::event_t;
private:
	com::threadpool m_threadpool;
	std::thread m_main_thread;
	std::multimap<time_point, timer> tasks;
	mem::data_pool<event_t, 50, 10, 10> m_event_pool;
	std::mutex mtx;
	std::condition_variable cv;
	std::atomic_size_t m_ntasks = { 0 };
	bool m_running = false;
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

	template<class Duration, class F, class... Args>
	timer attach(Duration&& tp, F&& f, Args&&... args)
	{
		std::lock_guard<std::mutex> lock(mtx);
		if (!m_running)
			return {0,nullptr};

		event_t* ev = m_event_pool.malloc();
		if (++ev->m_count == 0)
			ev->m_count = 1;
		ev->m_siCode = ev->m_count;
		ev->m_func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
		auto iter = tasks.emplace(std::make_pair(clock::now() + tp, timer{ ev->m_count, ev }));

		++m_ntasks;
		cv.notify_one();
		return iter->second;
	}

	// �ر������ύ �ȴ�ʣ��������� (����ʱ��ȷ��pool��������)
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

			time_point now = clock::now();
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