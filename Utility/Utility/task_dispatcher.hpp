/**
* @file task_dispatcher.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __TASK_DISPATCHER_HPP__
#define __TASK_DISPATCHER_HPP__

#include "com_thread_pool.hpp"
#include "task_controler.hpp"
#include "task_channel.hpp"
#include "mem_data_factory.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace task
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class dispatcher
{
	using task_info = controler::task_info;
	friend class controler;
	using channel_pool_t = mem::data_pool_s<task::channel, 50, 10, 10>;
public:
	dispatcher(void);
	~dispatcher(void);

	dispatcher(const dispatcher&) = delete;
	dispatcher& operator=(const dispatcher&) = delete;

	void start(std::uint32_t nworker = 1);
	void stop(void);
	static channel* malloc(void) { return m_pool.malloc(); }
	static bool free(channel* p) { return m_pool.free(p); }
private:
	void dispatch(task_info&& _task);
private:
	com::task_threadpool<task_info> m_workers;
	static channel_pool_t m_pool;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace task
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__TASK_DISPATCHER_HPP__