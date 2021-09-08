/**
* @file task_controler.cpp
*
* @author Hourui (liquidmonkey)
*/

#include "Utility/task_object.hpp"
#include "Utility/logger.hpp"

#define CONTROLER_LOG
#ifdef CONTROLER_LOG
#define CONTROLER_WARN(exp,fmt,...) if(!(exp)) Clog::warn(1,fmt,##__VA_ARGS__);
#else
#define CONTROLER_WARN(exp,fmt,...)
#endif

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace task
{
////////////////////////////////////////////////////////////////////////////////////////////////////
void controler::post_node(channel_node* node)
{
	channel* p_channel = node->m_parent;
	if (p_channel) {
		if (p_channel->post_node(node))
			post_node(p_channel);

		return;
	}
	m_dispatcher->dispatch({ node,this });
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void controler::dispatch_node(channel_node* node)
{
	CONTROLER_WARN(node->m_parent == nullptr, "node enter the channel in an unusual manner!");
	if (node->m_is_channel ? dispatch_channel(dynamic_cast<channel*>(node))
		: dynamic_cast<object_iface*>(node)->do_dispatch())
		post_node(node);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool controler::dispatch_channel(channel* p_channel)
{
	channel_node* node = p_channel->m_post_root;
	CONTROLER_WARN(node->m_parent == p_channel, "node change the channel in an unusual manner!");
	if (node->m_is_channel ? dispatch_channel(dynamic_cast<channel*>(node))
		: dynamic_cast<object_iface*>(node)->do_dispatch())
	{ 
		if(p_channel == node->m_parent)
			return p_channel->post_node(node);

		bool need_post = p_channel->pop_front();
		post_node(node);
		return need_post;
	}
	else
		return p_channel->pop_front();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool controler::dispatch_obj(object_iface* obj) { return dynamic_cast<object*>(obj)->exec_task(); }
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace msg
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 