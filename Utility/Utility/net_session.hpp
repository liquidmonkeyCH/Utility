/**
* @file net_session.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __NET_SESSION_HPP__
#define __NET_SESSION_HPP__

#include <mutex>
#include "net_socket.hpp"
#include "net_framework.hpp"
#include "net_io_service.hpp"
#include "msg_object.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace net
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class session_iface : public msg::object_iface
{
public:
	enum class reason : std::uint32_t {
		cs_none,
		cs_service_stop,
		cs_connect_timeout,
		cs_connect_peer_close,
		cs_send_buffer_overflow,
		cs_recv_buffer_overflow,
		cs_pack_error,
		cs_unpack_error,
		cs_handle_error,
		cs_null_handle,
		cs_handle_unknow_error,
	};
	enum class state{ none,connected,closing };
	friend class io_service_iface;
	template<class session_t, class handler_manager> friend class responder;
	template<class session_t, class handler_manager> friend class requester;
public:
	session_iface(void);
	virtual ~session_iface(void) = default;
public:
	fd_t get_fd(void);
	socket_iface* get_socket(void);
	void close(reason);
	bool is_connected(void);
protected:
	void set_connected(framework* parent,fd_t fd, sockaddr_storage* addr);
	virtual void handle_error(msg::err);
	//! for wrap
	virtual void clear(void) = 0;
	virtual void on_connect(void) = 0;
	virtual bool process_recv(net_size_t size) = 0;
	virtual bool process_send(net_size_t size) = 0;
	virtual void process_close(void) = 0;
protected:
	io_service_iface* m_io_service;
	framework* m_parent;
	std::atomic<state> m_state;

	socket_iface* m_socket;

	per_io_data m_recv_data;
	per_io_data m_send_data;

	reason m_close_reason;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
template<socket_type st, class pares_message_wrap>
class session_wrap : public session_iface
{
public:
	using socket_mode = socket_wrap<st> ;
	using message_t = pares_message_wrap;
	template<class session_t, class handler_manager> friend class responder;
	template<class session_t, class handler_manager> friend class requester;
	static constexpr std::size_t MAX_MSG_LEN = message_t::pre_block_size;
public:
	session_wrap(void);
	virtual ~session_wrap(void);
protected:
	void init_buffer(std::size_t recv_buffer_size, std::size_t send_buffer_size);

	void clear(void);
	void do_close(void);
	mem::message* get_message(void) { return &m_recv_buffer; }

	virtual void on_close(reason){}
	virtual void on_connect(void) {}

	bool process_recv(net_size_t size);
	bool process_send(net_size_t size);
	void process_close(void);
	void post_send(void);
public:
	bool send(const void* data, net_size_t len);
protected:
	socket_wrap<st> m_socket_impl;
	message_t m_recv_buffer;
	message_t m_send_buffer;
	std::mutex m_send_mutex;
	bool m_init_complete;
};
#define UTILITY_NET_SESSION_SEND_BEGIN(size)					\
	if (this->m_state != net::session_iface::state::connected)	\
			return false;										\
	std::lock_guard<std::mutex> lock(this->m_send_mutex);		\
	if (this->m_send_buffer.writable_size() < size){			\
		this->close(net::session_iface::reason::cs_send_buffer_overflow);			\
		return false;											\
	}

#define UTILITY_NET_SESSION_SEND(data,len,flag)			\
	bool flag = false;{									\
	char* p = nullptr;									\
	const char* packet = (const char*)data;				\
	net_size_t left = len;								\
	net_size_t size = len;								\
	while (left != 0) {									\
		size = left;									\
		p = this->m_send_buffer.write(size);			\
		memcpy(p, packet, size);						\
		flag |= this->m_send_buffer.commit_write(size);	\
		packet += size;									\
		left -= size;									\
	}}

#define UTILITY_NET_SESSION_SEND_END(flag)				\
	if (flag) this->post_send();

#include "net_session.inl"
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace net
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__NET_SESSION_HPP__