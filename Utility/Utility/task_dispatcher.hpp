/**
* @file task_dispatcher.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __TASK_DISPATCHER_HPP__
#define __TASK_DISPATCHER_HPP__

#include "com_thread_pool.hpp"
#include "task_object.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace task
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class dispatcher
{
	using task_info = object_iface::task_info;
	friend class controler;
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
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace task
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__TASK_DISPATCHER_HPP__