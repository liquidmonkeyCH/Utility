/**
* @file msg_object.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __MSG_OBJECT_HPP__
#define __MSG_OBJECT_HPP__

#include <atomic>
#include <cstdint>
#include <mutex>

#include "mem_message.hpp"
#include "msg_channel.hpp"
#include "msg_defines.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace msg
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class dispatcher;
class controler_iface;
////////////////////////////////////////////////////////////////////////////////////////////////////
class object_iface : public channel_node
{
public:
	object_iface(void) : channel_node(false) {};
	virtual ~object_iface(void) = default;

	object_iface(const object_iface&) = delete;
	object_iface& operator=(const object_iface&) = delete;

	virtual void handle_error(err) = 0;
	virtual void do_close(void) = 0;
	virtual mem::message* get_message(void) = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class pares_message_wrap>
class object : public object_iface
{
public:
	using message_t = pares_message_wrap;
	bool add_message_ex(const char* msg, std::size_t size)
	{
		if (m_message.writable_size() < size)
			return false;
		
		std::size_t len;
		char* p;
		bool b_post = false;
		do {
			len = size;
			p = m_message.write(len);
			memcpy(p, msg, len);
			size -= len;
		} while (size > 0);

		return true;
	}
protected:
	virtual void on_close(int) = 0;
	mem::message* get_message(void){ return &m_message; }
	void do_close(void) { on_close(m_close_reason);  m_message.clear(); }
protected:
	enum class state { none, connected, closing };
	std::atomic<state> m_state;
	int m_close_reason;
	controler_iface* m_controler;

	message_t m_message;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace msg
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__MSG_OBJECT_HPP__