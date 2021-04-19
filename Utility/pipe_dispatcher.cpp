/**
* @file msg_dispatcher.cpp
*
* @author Hourui (liquidmonkey)
*/

#include "Utility/msg_dispatcher.hpp"
#include "Utility/pipe_object.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace pipe
{
////////////////////////////////////////////////////////////////////////////////////////////////////
void dispatcher::task_info::exec(void)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
dispatcher::dispatcher(void)
{}
////////////////////////////////////////////////////////////////////////////////////////////////////
dispatcher::~dispatcher(void)
{
	stop();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
dispatcher::stop(void)
{
	m_workers.safe_stop();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
dispatcher::start(std::uint32_t nworker)
{
	assert(nworker > 0);
	m_workers.init(nworker);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
dispatcher::dispatch(task_info&& _task)
{
	m_workers.schedule(std::move(_task));
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace msg
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 