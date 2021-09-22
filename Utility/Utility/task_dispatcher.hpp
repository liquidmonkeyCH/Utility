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
	struct task_info
	{
		channel_node* m_node;
		dispatcher* m_dispatcher;
		inline void exec(void) { m_dispatcher->dispatch_node(m_node); }
	};
	friend class controler;
	using channel_pool_t = mem::data_pool_s<task::channel,10>;
public:
	dispatcher(void);
	~dispatcher(void);

	dispatcher(const dispatcher&) = delete;
	dispatcher& operator=(const dispatcher&) = delete;

	void start(std::uint32_t nworker = 1);
	void stop(void);
	inline static channel* malloc(void) { return m_pool.malloc(); }
	inline static bool free(channel* p) { return m_pool.free(p); }
private:
	inline void dispatch(task_info&& _task) { m_workers.schedule(std::move(_task)); }

	void post_node(channel_node* node);
	void dispatch_node(channel_node* node);
	bool dispatch_channel(channel* p_channel);
private:
	com::task_threadpool<task_info> m_workers;
	static channel_pool_t m_pool;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace task
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__TASK_DISPATCHER_HPP__