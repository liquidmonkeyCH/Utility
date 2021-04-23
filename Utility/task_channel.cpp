/**
* @file task_channel.cpp
*
* @author Hourui (liquidmonkey)
*/

#include "Utility/task_channel.hpp"
#include "Utility/logger.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace task
{
////////////////////////////////////////////////////////////////////////////////////////////////////
void channel_node::clear(void)
{
	m_prev = nullptr;
	m_next = nullptr;
	m_parent = nullptr;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void channel_node::leave_channel(void)
{
	m_parent = nullptr;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
channel::channel(void)
: channel_node(true)
, m_post_root(nullptr)
, m_post_tail(nullptr)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
channel::~channel(void)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void channel::attach(channel_node* node)
{
	if (node == this)
		Clog::error_throw(errors::logic, "can not attach self!");

	channel* exp = nullptr;
	if (!node->m_parent.compare_exchange_strong(exp,this))
		Clog::error_throw(errors::logic, "node already have channel!");
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void channel::detach(channel_node* node)
{
	channel* exp = this;
	if (!node->m_parent.compare_exchange_strong(exp, nullptr))
		Clog::error_throw(errors::logic, "node not belong to this channel!");
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool channel::post_node(channel_node* node)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	if (node == m_post_root) {			// 重新投递消息(由dispatch线程投递)
		if (nullptr == node->m_next)	// 唯一子节点
			return true;

		// 从root移动到tail
		m_post_root = node->m_next;
		m_post_root->m_prev = nullptr;
		node->m_prev = m_post_tail;
		node->m_next = nullptr;
		m_post_tail->m_next = node;
		m_post_tail = node;
		return true;
	}

	// 新进节点
	node->m_prev = m_post_tail;
	node->m_next = nullptr;
	m_post_tail ? m_post_tail->m_next = node : m_post_root = node;
	m_post_tail = node;

	return node == m_post_root;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool channel::pop_front(void)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_post_root = m_post_root->m_next;
	if (m_post_root) {
		m_post_root->m_prev = nullptr;
		return true;
	}
	m_post_tail = nullptr;
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace msg
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 