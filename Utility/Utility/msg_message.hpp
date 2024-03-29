/**
* @file msg_message.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __MSG_MESSAGE_HPP__
#define __MSG_MESSAGE_HPP__

#include "msg_defines.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace net 
{ 
	enum class socket_type; 
	template<net::socket_type st, class pares_message_wrap> class session_wrap;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace msg
{
////////////////////////////////////////////////////////////////////////////////////////////////////
enum class state { ok, pending, error, bad };
template<class message_wrap, class handler_manager> class controler;
template<class pares_message_wrap> class session;
template<class buffer_t, net_size_t msg_len_max = buffer_t::pre_block_size>
struct buffer : public buffer_t
{
	static constexpr net_size_t max_message_len = msg_len_max;
	using buffer_type = buffer_t;
	buffer(void) {
		static_assert(max_message_len > 0, "max_message_len out of range!");
		static_assert(max_message_len <= buffer_type::max_message_len, "max_message_len out of bound!");
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace __impl
{
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class msg_buffer>
class message : public msg_buffer
{
public:
	using buffer_type = typename msg_buffer::buffer_type;
	message(void) :m_size(0), m_good(true){}
	virtual ~message(void) = default;

	template<class message_wrap, class handler_manager> friend class msg::controler;
	template<net::socket_type st, class pares_message_wrap> friend class net::session_wrap;
	template<class pares_message_wrap> friend class msg::session;
protected:
	void commit(void)
	{
		this->commit_read(m_size);
		m_size = 0;
		this->reset();
		this->set_read_limit(0);
	}

	void clear(void)
	{
		m_size = 0;
		this->reset();
		this->set_read_limit(0);
		buffer_type::clear();
		m_good = true;
	}

	bool go_bad(void)
	{
		std::lock_guard<std::mutex> lock(this->m_mutex);
		m_good = false;
		return this->buffer_type::_commit_write(0);
	}

	bool is_good(void)
	{
		std::lock_guard<std::mutex> lock(this->m_mutex);
		return m_good;
	}

	bool commit_recv(net_size_t size)
	{
		std::lock_guard<std::mutex> lock(this->m_mutex);
		if (!m_good) return false;
		return this->buffer_type::_commit_write(size);
	}
protected:
	bool readable_state(net_size_t& exp)
	{
		std::lock_guard<std::mutex> lock(this->m_mutex);
		if (!m_good) return false;
		exp = this->buffer_type::_readable_size(exp);
		return true;
	}
protected:
	net_size_t m_size;
	bool m_good;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace msg
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__MSG_MESSAGE_HPP__