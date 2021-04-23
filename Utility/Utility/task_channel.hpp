/**
* @file task_channel.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __TASK_CHANNEL_HPP__
#define __TASK_CHANNEL_HPP__

#include <thread>
#include <atomic>
#include <mutex>

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace task
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class channel;
////////////////////////////////////////////////////////////////////////////////////////////////////
class channel_node
{
public:
	friend class controler;
	friend class channel;

	channel_node(void) = delete;
	channel_node(bool flag) :m_is_channel(flag), m_prev(nullptr), m_next(nullptr), m_parent(nullptr) {}
	virtual ~channel_node(void) { clear(); }
protected:
	void clear(void);
	void leave_channel(void);
protected:
	const bool m_is_channel;
	channel_node* m_prev;
	channel_node* m_next;
	std::atomic<channel*> m_parent;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class channel : public channel_node
{
public:
	channel(void);
	virtual ~channel(void);
	
	friend class controler;
public:
	void attach(channel_node* node);
	void detach(channel_node* node);
private:
	bool post_node(channel_node* node);
	bool pop_front(void);
private:
	std::mutex m_mutex;
	channel_node* m_post_root;
	channel_node* m_post_tail;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace task
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__TASK_CHANNEL_HPP__