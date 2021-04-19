/**
* @file pipe_object.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __PIPE_OBJECT_HPP__
#define __PIPE_OBJECT_HPP__

#include <functional>
#include <mutex>
#include <queue>
#include "com_thread_pool.hpp"
namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace pipe
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class object;
class dispatcher
{
public:
	struct task_info
	{
		object* m_obj;
		void exec(void);
	};
	friend class object;
public:
	dispatcher(void);
	~dispatcher(void);

	dispatcher(const dispatcher&) = delete;
	dispatcher& operator=(const dispatcher&) = delete;

	void start(std::uint32_t nworker = 1);
	void stop(void);
private:
	void dispatch(task_info&& _task);
private:
	com::task_threadpool<task_info> m_workers;
};
class controler_iface;
////////////////////////////////////////////////////////////////////////////////////////////////////
class object
{
public:
	using task_t = std::function<void(object*)>;
	object(void){};
	virtual ~object(void) = default;

	inline void init(dispatcher* _dispatcher) {
		m_dispatcher = _dispatcher;
	}

	object(const object&) = delete;
	object& operator=(const object&) = delete;

	void post(task_t&& task) {
		bool res = false; {
			std::lock_guard<std::mutex> lock(m_mutex);
			res = tasks.empty();
			tasks.push(std::move(task));
		}
		if (res)
			m_dispatcher->dispatch({this});
	}

	task_t get_task(void) {
		std::lock_guard<std::mutex> lock(m_mutex);
		if (tasks.empty())
			return task_t();
		task_t task(std::move(tasks.front()));
		tasks.pop();
		return task;
	}
private:
	std::mutex m_mutex;
	std::queue<task_t> tasks;
	dispatcher* m_dispatcher;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace pipe
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__MSG_OBJECT_HPP__