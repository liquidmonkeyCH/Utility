/**
* @file net_session.inl
*
* @author Hourui (liquidmonkey)
*/
////////////////////////////////////////////////////////////////////////////////////////////////////
template<socket_type st, class pares_message_wrap>
session_wrap<st, pares_message_wrap>::session_wrap(void):m_init_complete(false)
{
	m_socket = &m_socket_impl;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<socket_type st, class pares_message_wrap>
session_wrap<st, pares_message_wrap>::~session_wrap(void)
{
	m_socket_impl.close();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<socket_type st, class pares_message_wrap>
void session_wrap<st, pares_message_wrap>::init(std::size_t recv_buffer_size, std::size_t send_buffer_size, msg::controler_iface* controler)
{
	if (m_init_complete) return;
	m_recv_buffer.init(recv_buffer_size);
	m_send_buffer.init(send_buffer_size);

	m_controler = controler;
	m_recv_data.m_buffer.len = MAX_MSG_LEN;
	m_recv_data.m_buffer.buf = m_recv_buffer.write(m_recv_data.m_buffer.len);
	m_init_complete = true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<socket_type st, class pares_message_wrap>
void session_wrap<st, pares_message_wrap>::clear(void)
{
	this->m_close_reason = reason::cs_none;

	m_recv_buffer.clear();
	m_send_buffer.clear();

	m_recv_data.m_buffer.len = MAX_MSG_LEN;
	m_recv_data.m_buffer.buf = m_recv_buffer.write(m_recv_data.m_buffer.len);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<socket_type st, class pares_message_wrap>
void session_wrap<st, pares_message_wrap>::do_close(void)
{
	on_close(this->m_close_reason);
	this->leave_channel();
	m_parent->on_close_session(this);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<socket_type st, class pares_message_wrap>
bool session_wrap<st, pares_message_wrap>::process_send(net_size_t size)
{
	if (m_state != state::connected)
		return false;

	m_send_buffer.commit_read(size);
	size = MAX_MSG_LEN;
	const char* p = m_send_buffer.read(size);

	if (size == 0 || m_state != state::connected)
		return false;

	m_send_data.m_buffer.buf = const_cast<char*>(p);
	m_send_data.m_buffer.len = size;
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<socket_type st, class pares_message_wrap>
void session_wrap<st, pares_message_wrap>::process_close(void)
{
	if(m_recv_buffer.go_bad())
		m_controler->post_request(this, &m_recv_buffer);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<socket_type st, class pares_message_wrap>
bool session_wrap<st, pares_message_wrap>::process_recv(net_size_t size)
{
	if (m_state != state::connected)
		return false;

	if (m_recv_buffer.commit_recv(size))
		m_controler->post_request(this, &m_recv_buffer);

	m_recv_data.m_buffer.len = MAX_MSG_LEN;
	m_recv_data.m_buffer.buf = m_recv_buffer.write(m_recv_data.m_buffer.len);

	if (m_recv_data.m_buffer.len == 0)
	{
		close(reason::cs_recv_buffer_overflow);
		return false;
	}

	return (m_state == state::connected);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<socket_type st, class pares_message_wrap>
void session_wrap<st, pares_message_wrap>::post_send(void)
{
	m_send_data.m_buffer.len = MAX_MSG_LEN;
	m_send_data.m_buffer.buf =
		const_cast<char*>(m_send_buffer.read(m_send_data.m_buffer.len));
	if (m_state == state::connected)
		m_io_service->post_send_event(&m_send_data);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<socket_type st, class pares_message_wrap>
bool session_wrap<st, pares_message_wrap>::send(const void* data, net_size_t len)
{
	UTILITY_NET_SESSION_SEND_BEGIN(len);
	UTILITY_NET_SESSION_SEND(data,len);
	UTILITY_NET_SESSION_SEND_END();
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////