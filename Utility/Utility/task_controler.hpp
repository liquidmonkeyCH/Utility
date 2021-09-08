/**
* @file task_controler.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __TASK_CONTROLER_HPP__
#define __TASK_CONTROLER_HPP__

#include "logger.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace task
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class dispatcher;
class channel_node;
class channel;
class object_iface;
class controler
{
public:
	struct task_info
	{
		channel_node* m_node;
		controler* m_controler;
		inline void exec(void) { m_controler->dispatch_node(m_node); }
	};
	friend class object_iface;
	friend struct task_info;
	controler(void) = default;
	~controler(void) = default;

	controler(const controler&) = delete;
	controler& operator=(const controler&) = delete;

	inline void init(dispatcher* _dispatcher) {
		if (m_dispatcher)
			Clog::error_throw(errors::logic, "controler initialized!");
		m_dispatcher = _dispatcher;
	}
protected:
	void post_node(channel_node* node);
	void dispatch_node(channel_node* node);
	bool dispatch_channel(channel* p_channel);

	virtual bool dispatch_obj(object_iface* obj);
	virtual void post_request(channel_node* object) { post_node(object); }
protected:
	dispatcher* m_dispatcher = nullptr;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace task
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__TASK_CONTROLER_HPP__