/**
* @file msg_session.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __MSG_SESSION_HPP__
#define __MSG_SESSION_HPP__

#include "msg_object.hpp"
#include "msg_controler.hpp"
#include "mem_message.hpp"

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
	session(const session&) = delete;
	session& operator=(const session&) = delete;

	void init(std::size_t buffer_size, task::controler* controler);
	bool add_message_ex(const char* msg, std::size_t len);
protected:
	mem::message* get_message(void) { return &m_message; }

	void close(int st);
	void do_close(void) { leave_channel(); on_close(m_close_reason);  m_message.clear(); }
	virtual void on_close(int) = 0;
protected:
	enum class state { none, running, closing };
	std::mutex m_mutex;
	message_t m_message;
	std::atomic<state> m_state;
	int m_close_reason;
	std::atomic_bool m_init_complete;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class pares_message_wrap>
using session_channel = task::wrap::object_channel<session<pares_message_wrap>>;
////////////////////////////////////////////////////////////////////////////////////////////////////
#define UTILITY_MSG_SESSION_ADD_MESSAGE_BEGIN(_len)			\
if (this->m_state != state::running)						\
		return false;										\
std::lock_guard<std::mutex> lock(this->m_mutex);			\
if (this->m_message.writable_size() < _len){				\
	return false;											\
}															\
	bool _flag = false;										\
	char* _p = nullptr;										\
	const char* _packet;									\
	net_size_t _left, _size;

#define UTILITY_MSG_SESSION_ADD_MESSAGE(_data,_len)	\
_packet = (const char*)_data;						\
_left = _len;										\
_size = _len;										\
while (_left != 0) {								\
	_size = _left;									\
	_p = this->m_message.write(_size);				\
	memcpy(_p, _packet, _size);						\
	if(this->m_message.commit_write(_size))			\
		_flag = true;								\
	_packet += _size;								\
	_left -= _size;									\
}

#define UTILITY_MSG_SESSION_ADD_MESSAGE_END()		\
if (_flag) post_request();

template<class pares_message_wrap>
session<pares_message_wrap>::session(void)
	: m_state(state::none)
	, m_close_reason(0)
	, m_init_complete{ false }
{
}

template<class pares_message_wrap>
session<pares_message_wrap>::~session(void)
{
}

template<class pares_message_wrap>
void session<pares_message_wrap>::init(std::size_t buffer_size, task::controler* controler)
{
	bool exp = false;
	if (!m_init_complete.compare_exchange_strong(exp,true)) return;
	m_message.init(buffer_size);
	m_controler = controler;
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
		post_request();
}

template<class pares_message_wrap>
bool session<pares_message_wrap>::add_message_ex(const char* msg, std::size_t len)
{
	UTILITY_MSG_SESSION_ADD_MESSAGE_BEGIN(len);
	UTILITY_MSG_SESSION_ADD_MESSAGE(msg, len);
	UTILITY_MSG_SESSION_ADD_MESSAGE_END();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace msg
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__MSG_SESSION_HPP__