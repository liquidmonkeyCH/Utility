/**
* @file task_controler.cpp
*
* @author Hourui (liquidmonkey)
*/

#include "Utility/task_controler.hpp"
#include "Utility/task_dispatcher.hpp"
#include "Utility/task_channel.hpp"
#include "Utility/task_object.hpp"
#include "Utility/com_guard.hpp"

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
	if (node->m_is_channel ? dispatch_channel(dynamic_cast<channel*>(node))
		: dispatch_obj(dynamic_cast<object_iface*>(node)))
		post_node(node);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool controler::dispatch_channel(channel* p_channel)
{
#ifndef NDEBUG
	com::guard<channel_node*> guard(p_channel, &channel_node::set_thread_id, &channel_node::reset_thread_id);
#endif
	channel_node* node = p_channel->m_post_root;
	if (node->m_is_channel ? dispatch_channel(dynamic_cast<channel*>(node))
		: dispatch_obj(dynamic_cast<object_iface*>(node)))
	{ 
		if(p_channel == node->m_parent)
			return p_channel->post_node(node);
		else
			post_node(node);
	}
	return p_channel->pop_front();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool controler::dispatch_obj(object_iface* obj) {
#ifndef NDEBUG
	com::guard<channel_node*> guard(obj, &channel_node::set_thread_id, &channel_node::reset_thread_id);
#endif
	return dynamic_cast<object*>(obj)->exec_task();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace msg
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 