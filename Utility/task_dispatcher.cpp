/**
* @file task_dispatcher.cpp
*
* @author Hourui (liquidmonkey)
*/

#include "Utility/task_dispatcher.hpp"
#include "Utility/task_object.hpp"

#define DISPATCHER_LOG
#ifdef DISPATCHER_LOG
#define DISPATCHER_WARN(exp,fmt,...) if(!(exp)) Clog::warn(1,fmt,##__VA_ARGS__);
#else
#define DISPATCHER_WARN(exp,fmt,...)
#endif

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace task
{
////////////////////////////////////////////////////////////////////////////////////////////////////
dispatcher::channel_pool_t dispatcher::m_pool;
////////////////////////////////////////////////////////////////////////////////////////////////////
dispatcher::dispatcher(void){ dispatcher::channel_pool_t::super::set_cache(-1); }
////////////////////////////////////////////////////////////////////////////////////////////////////
dispatcher::~dispatcher(void){
	stop();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
dispatcher::stop(void){
	m_workers.safe_stop();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
dispatcher::start(std::uint32_t nworker){
	assert(nworker > 0);
	m_workers.init(nworker);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void dispatcher::post_node(channel_node* node)
{
	channel* p_channel = node->m_parent;
	if (p_channel) {
		if (p_channel->post_node(node))
			post_node(p_channel);

		return;
	}
	dispatch({ node,this });
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void dispatcher::dispatch_node(channel_node* node)
{
	DISPATCHER_WARN(node->m_parent == nullptr, "node enter the channel in an unusual manner!");
	if (node->m_is_channel ? dispatch_channel(dynamic_cast<channel*>(node))
		: dynamic_cast<object_iface*>(node)->do_dispatch())
		post_node(node);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool dispatcher::dispatch_channel(channel* p_channel)
{
	channel_node* node = p_channel->m_post_root;
	DISPATCHER_WARN(node->m_parent == p_channel, "node change the channel in an unusual manner!");
	if (node->m_is_channel ? dispatch_channel(dynamic_cast<channel*>(node))
		: dynamic_cast<object_iface*>(node)->do_dispatch())
	{
		if (p_channel == node->m_parent)
			return p_channel->post_node(node);

		bool need_post = p_channel->pop_front();
		post_node(node);
		return need_post;
	}
	else
		return p_channel->pop_front();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace task
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 