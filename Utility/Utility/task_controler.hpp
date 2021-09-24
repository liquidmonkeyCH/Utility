/**
* @file task_controler.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __TASK_CONTROLER_HPP__
#define __TASK_CONTROLER_HPP__

#include "task_dispatcher.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace task
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class object_iface;
class channel_node;
class controler
{
public:
	friend class object_iface;
	controler(void) = default;
	~controler(void) = default;

	controler(const controler&) = delete;
	controler& operator=(const controler&) = delete;

	void init(dispatcher* _dispatcher);
protected:
	virtual bool dispatch_obj(object_iface* obj);
	virtual inline void post_request(channel_node* object) { m_dispatcher->post_node(object); }
protected:
	dispatcher* m_dispatcher = nullptr;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace task
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__TASK_CONTROLER_HPP__