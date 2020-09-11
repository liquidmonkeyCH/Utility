/**
* @file msg_session.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __MSG_SESSION_HPP__
#define __MSG_SESSION_HPP__

#include "msg_object.hpp"
#include "msg_controler.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace msg
{
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class pares_message_wrap>
class session : public object_iface
{
public:
	using message_t = pares_message_wrap;
public:
	session(void);
	virtual ~session(void);

	void init(std::size_t buffer_size, controler_iface* controler);
	bool add_message_ex(const char* msg, std::size_t len);
protected:
	mem::message* get_message(void) { return &m_message; }

	void close(int st);
	void do_close(void) { on_close(m_close_reason);  m_message.clear(); }
	virtual void on_close(int) = 0;

	void post_send(void) { m_controler->post_request(this, &m_message); }
protected:
	enum class state { none, running, closing };
	std::mutex m_mutex;
	message_t m_message;
	std::atomic<state> m_state;
	controler_iface* m_controler;
	int m_close_reason;
	bool m_init_complete;
};
#define UTILITY_MSG_SESSION_ADD_MESSAGE_BEGIN(size)				\
	if (this->m_state != state::running)						\
			return false;										\
	std::lock_guard<std::mutex> lock(this->m_mutex);			\
	if (this->m_message.writable_size() < size){				\
		return false;											\
	}

#define UTILITY_MSG_SESSION_ADD_MESSAGE(data,len,flag)	\
	char* p = nullptr;									\
	const char* packet = (const char*)data;				\
	net_size_t left = len;								\
	net_size_t size = len;								\
	bool flag = false;									\
	do {												\
		size = left;									\
		p = this->m_message.write(size);				\
		memcpy(p, packet, size);						\
		flag |= this->m_message.commit_write(size);		\
		packet += size;									\
		left -= size;									\
	} while (left != 0);

#define UTILITY_MSG_SESSION_ADD_MESSAGE_END(flag)		\
	if (flag) this->post_send();

template<class pares_message_wrap>
session<pares_message_wrap>::session(void)
	: m_state(state::none)
	, m_controler(nullptr)
	, m_close_reason(0)
	, m_init_complete(false)
{
}

template<class pares_message_wrap>
session<pares_message_wrap>::~session(void)
{
}

template<class pares_message_wrap>
void session<pares_message_wrap>::init(std::size_t buffer_size, controler_iface* controler)
{
	if (m_init_complete) return;
	m_message.init(buffer_size);
	m_controler = controler;
	m_init_complete = true;
}

template<class pares_message_wrap>
void session<pares_message_wrap>::close(int st)
{
	state exp = state::running;
	if (!m_state.compare_exchange_strong(exp, state::closing))
		return;

	m_close_reason = st;
	m_state = state::none;
	if (m_message.go_bad())
		m_controler->post_request(this, &m_message);
}

template<class pares_message_wrap>
bool session<pares_message_wrap>::add_message_ex(const char* msg, std::size_t len)
{
	UTILITY_MSG_SESSION_ADD_MESSAGE_BEGIN(len);
	UTILITY_MSG_SESSION_ADD_MESSAGE(msg, len, b_send);
	UTILITY_MSG_SESSION_ADD_MESSAGE_END(b_send);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace msg
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__MSG_SESSION_HPP__