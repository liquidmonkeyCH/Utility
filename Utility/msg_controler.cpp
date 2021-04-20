/**
* @file msg_controler.cpp
*
* @author Hourui (liquidmonkey)
*/

#include "Utility/msg_controler.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace msg
{
////////////////////////////////////////////////////////////////////////////////////////////////////
void controler_iface::post_node(channel_node* node)
{
	channel* p_channel = node->m_parent;
	if (p_channel) {
		if (p_channel->post_node(node))
			post_node(p_channel);

		return;
	}

	controler::post_request(node);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void controler_iface::dispatch_node(channel_node* node)
{
	if (node->m_is_channel ? dispatch_channel(dynamic_cast<channel*>(node))
		: dispatch_obj(dynamic_cast<object_iface*>(node)))
		post_node(node);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool controler_iface::dispatch_channel(channel* p_channel)
{
#ifndef NDEBUG
	com::guard<channel_node*> guard(p_channel, &channel_node::set_thread_id, &channel_node::reset_thread_id);
#endif
	channel_node* node = p_channel->front();
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
}//namespace msg
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 