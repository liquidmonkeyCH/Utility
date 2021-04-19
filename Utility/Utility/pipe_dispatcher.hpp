/**
* @file pipe_dispatcher.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __PIPE_DISPATCHER_HPP__
#define __PIPE_DISPATCHER_HPP__

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
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace pipe
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__PIPE_DISPATCHER_HPP__