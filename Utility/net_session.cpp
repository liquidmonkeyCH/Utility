/**
* @file net_session.cpp
*
* @author Hourui (liquidmonkey)
*/

#include "Utility/net_session.hpp"
#include "Utility/net_io_service.hpp"
#include "Utility/net_framework.hpp"

//#define SESSION_LOG
#ifdef SESSION_LOG
#define SESSION_DEBUG(fmt,...) NET_DEBUG(fmt,##__VA_ARGS__)
#else
#define SESSION_DEBUG(fmt,...)
#endif

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace net
{
////////////////////////////////////////////////////////////////////////////////////////////////////
session_iface::session_iface(void)
: m_io_service(nullptr)
, m_parent(nullptr)
, m_state(state::none)
, m_socket(nullptr)
, m_recv_data()
, m_send_data()
, m_close_reason(reason::cs_none)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
fd_t
session_iface::get_fd(void)
{
	return m_socket->get_fd();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
socket_iface*
session_iface::get_socket(void)
{
	return m_socket;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
session_iface::close(reason st)
{
	state exp = state::connected;
	if (!m_state.compare_exchange_strong(exp, state::closing))
		return;

	Clog::info("session close:%d", static_cast<std::uint32_t>(st));
	m_close_reason = st;
	m_io_service->untrack_session(this);
	m_socket->close();
	m_state = state::none;
	process_close();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void 
session_iface::handle_error(msg::err ret)
{
	switch (ret)
	{
	case msg::err::logic:
		close(reason::cs_handle_error);
		break;
	case msg::err::null_handle:
		close(reason::cs_null_handle);
		break;
	case msg::err::pack:
		close(reason::cs_pack_error);
		break;
	case msg::err::unpack:
		close(reason::cs_unpack_error);
		break;
	default:
		close(reason::cs_handle_unknow_error);
		break;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool
session_iface::is_connected(void)
{
	return m_state == state::connected;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void
session_iface::set_connected(framework* parent, fd_t fd, sockaddr_storage* addr)
{
	m_state = state::connected;
	m_parent = parent;
	on_connect();
	if (fd == INVALID_SOCKET)
		return;

	m_socket->set_fd(fd, addr);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace net
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 