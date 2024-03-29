/**
* @file net_requester.inl
*
* @author Hourui (liquidmonkey)
*/
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class session_t, class handler_manager>
void requester<session_t, handler_manager>::init(io_service_iface* io_service, dispatcher_t* _dispatcher)
{
	if (m_io_service)
		Clog::error_throw(errors::logic, "client initialized!");

	m_io_service = io_service;
	m_controler.init(_dispatcher);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class session_t, class handler_manager>
requester_iface::state requester<session_t, handler_manager>::start(const char* host, std::uint16_t port, std::uint32_t timeout_msecs)
{
	int exp = static_cast<int>(state::none);
	if (!m_state.compare_exchange_strong(exp, static_cast<int>(state::starting)))
		return static_cast<state>(exp);

	framework::net_init();
	on_start();

	if (connect(host, port, timeout_msecs))
		return state::connected;
	
	framework::net_free();
	m_state = static_cast<int>(state::none);
	return state::timeout;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class session_t, class handler_manager>
bool requester<session_t, handler_manager>::connect(const char* host, std::uint16_t port, std::uint32_t timeout_msecs)
{
	int exp = static_cast<int>(state::starting);
	if (!m_state.compare_exchange_strong(exp, static_cast<int>(state::connecting)))
		return false;

	try{
		if (m_session.m_socket->connect(host, port, timeout_msecs)){
			m_session.set_connected(this, INVALID_SOCKET, nullptr);
			m_session.init(m_recv_buffer_size, m_send_buffer_size,&m_controler);
			m_session.m_socket->set_blocking(false);
			m_session.m_id = 1;
			m_io_service->track_session(&m_session);
			m_can_stop = std::promise<bool>();
			m_state = static_cast<int>(state::connected);
			return true;
		}
	}
	catch (utility_error& e){
		Clog::error(e.what());
		throw(e);
	}
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class session_t, class handler_manager>
void requester<session_t, handler_manager>::stop(void)
{
	int exp = static_cast<int>(state::connected);
	if (!m_state.compare_exchange_strong(exp, static_cast<int>(state::stopping)))
		return;

	m_session.close(session_iface::reason::cs_service_stop);
	on_stop();

	framework::net_free();
	m_state = static_cast<int>(state::none);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class session_t, class handler_manager>
void requester<session_t, handler_manager>::join(void)
{
	m_can_stop.get_future().get();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class session_t, class handler_manager>
void requester<session_t, handler_manager>::on_close_session(session_iface* session)
{
	(void)session;
	session->m_id = 0;
	m_can_stop.set_value(true);
}
////////////////////////////////////////////////////////////////////////////////////////////////////