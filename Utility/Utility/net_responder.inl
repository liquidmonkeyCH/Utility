/**
* @file net_responder.inl
*
* @author Hourui (liquidmonkey)
*/
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class session_t, class handler_manager>
void responder<session_t, handler_manager>::init(size_t max_session, io_service_iface* io_service, dispatcher_t* _dispatcher)
{
	if (m_io_service)
		Clog::error_throw(errors::logic, "server initialized!");

	m_session_pool.init(max_session);
	m_accept_data.init(10);

	m_io_service = io_service;
	m_controler.init(_dispatcher);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class session_t, class handler_manager>
void responder<session_t, handler_manager>::start(const char* host, std::uint16_t port)
{
	bool exp = false;
	if (!m_running.compare_exchange_strong(exp, true))
		Clog::error_throw(errors::logic, "server already running!");

	framework::net_init();
	on_start();

	m_socket->bind(host, port);
	m_socket->listen();
	m_session_index = std::uint64_t(port) << 48;

	m_running = true;
	m_can_stop = std::promise<bool>();
	
	m_io_service->track_server(this);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class session_t, class handler_manager>
void responder<session_t, handler_manager>::stop(void)
{
	bool exp = true;
	if (!m_running.compare_exchange_strong(exp, false))
		return;

	m_io_service->untrack_server(this);
	m_socket->close();

	for (auto it = m_accept_data.used_begin(); it != m_accept_data.used_end(); ++it)
	{
		if (it->m_fd != INVALID_SOCKET)
			m_socket->close_fd(it->m_fd);
	}
	m_accept_data.clear();

	std::future<bool> stop_wait;
	m_session_mutex.lock();

	for (auto it = m_session_pool.used_begin(); it != m_session_pool.used_end(); ++it)
	{
		if (!stop_wait.valid())
			stop_wait = m_can_stop.get_future();

		it->close(session_iface::reason::cs_service_stop);
	}
	m_session_mutex.unlock();

	// wait all session close!
	if (stop_wait.valid())
		stop_wait.get();

	m_session_pool.clear();

	on_stop();
	framework::net_free();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class session_t, class handler_manager>
session_t* responder<session_t, handler_manager>::get_session(void)
{
	std::lock_guard<std::mutex> lock(m_session_mutex);
	if (!m_running)
		return nullptr;

	session_t* session = m_session_pool.malloc();
	if (session) 
	{
		if (++m_session_index == 0)
			m_session_index = 1;

		session->m_id = m_session_index;
		m_session_map.emplace(m_session_index, session);
	}

	return session;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class session_t, class handler_manager>
session_t* responder<session_t, handler_manager>::get_session(std::uint64_t id)
{
	std::lock_guard<std::mutex> lock(m_session_mutex);
	if (!m_running || !id)
		return nullptr;

	auto iter = m_session_map.find(id);
	if (iter == m_session_map.end())
		return nullptr;

	return iter->second;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class session_t, class handler_manager>
void responder<session_t, handler_manager>::process_accept(per_io_data* data, sockaddr_storage* addr, session_iface** se)
{
	if (!on_accept())
	{
		m_socket->close_fd(data->m_fd);
		*se = nullptr;
		return;
	}

	session_t* session = get_session();
	if (!session)
	{
		m_socket->close_fd(data->m_fd);
	}
	else
	{
		session->set_connected(this, data->m_fd, addr);
		session->init(m_recv_buffer_size, m_send_buffer_size, &m_controler);
		session->m_socket->set_blocking(false);
	}
	*se = session;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class session_t, class handler_manager>
void responder<session_t, handler_manager>::on_close_session(session_iface* session)
{
	std::lock_guard<std::mutex> lock(m_session_mutex);
	m_session_map.erase(session->m_id);
	session->m_id = 0;
	m_session_pool.free(dynamic_cast<session_t*>(session));

	on_disconnect();
	if (!m_running && m_session_pool.used() == 0)
		m_can_stop.set_value(true);
}
////////////////////////////////////////////////////////////////////////////////////////////////////