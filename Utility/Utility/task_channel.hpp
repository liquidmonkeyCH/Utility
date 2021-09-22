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
namespace mem { template<class, std::uint64_t> class data_factory; }
namespace task
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class channel;
////////////////////////////////////////////////////////////////////////////////////////////////////
class channel_node
{
public:
	friend class controler;
	friend class dispatcher;

	channel_node(void) = delete;
	channel_node(bool flag) :m_is_channel(flag), m_prev(nullptr), m_next(nullptr), m_parent(nullptr) {}
	virtual ~channel_node(void) { clear(); }

	inline virtual void enter_channel(channel* p_channel) { m_parent = p_channel; }
	inline virtual void leave_channel(void) { m_parent = nullptr; }
protected:
	inline void clear(void){ m_prev = nullptr;m_next = nullptr;m_parent = nullptr; }
protected:
	const bool m_is_channel;
	channel_node* m_prev;
	channel_node* m_next;
	std::atomic<channel*> m_parent;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class channel : public channel_node
{
protected:
	channel(void);
	virtual ~channel(void);
	
	friend class controler;
	friend class dispatcher;
	template<class,std::uint64_t>
	friend class mem::data_factory;
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