/**
* @file msg_handler_manager_deque.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __MSG_HANDLER_MANAGER_DEQUE_HPP__
#define __MSG_HANDLER_MANAGER_DEQUE_HPP__

#include <deque>
#include "msg_defines.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace msg
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class handler_manager_alone
{
public:
	handler_manager_alone(void) :m_handler(nullptr){}
	virtual ~handler_manager_alone(void) = default;

	handler_manager_alone(const handler_manager_alone&) = delete;
	handler_manager_alone& operator=(const handler_manager_alone&) = delete;
public:
	handler_t get_handle(mem::message* msg = nullptr) { (void)msg; return m_handler; }
	bool attach(handler_t handle) { m_handler = handle; return true; }
private:
	handler_t m_handler;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class handler_manager_deque
{
public:
	using map_t = std::deque<handler_t>;
public:
	handler_manager_deque(void) = default;
	virtual ~handler_manager_deque(void) = default;

	handler_manager_deque(const handler_manager_deque&) = delete;
	handler_manager_deque& operator=(const handler_manager_deque&) = delete;
public:
	handler_t get_handle(mem::message* msg = nullptr);
	bool attach(handler_t handle);
	bool detach(void);
private:
	map_t m_map;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
inline handler_t 
handler_manager_deque::get_handle(mem::message* msg)
{
	(void)msg;
	if (m_map.empty())
		return nullptr;

	return m_map.front();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool 
handler_manager_deque::attach(handler_t handle)
{
	m_map.push_back(handle);
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool
handler_manager_deque::detach(void)
{
	m_map.pop_front();
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace task
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__MSG_HANDLER_MANAGER_DEQUE_HPP__