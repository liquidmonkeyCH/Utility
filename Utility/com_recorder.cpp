/**
* @file com_recorder.cpp
*
* @author Hourui (liquidmonkey)
*/

#include "Utility/com_recorder.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace com
{
////////////////////////////////////////////////////////////////////////////////////////////////////
void recorder::noter::write(const char* src, size_t len) {
	assert(m_parent && m_parent->m_state == recorder::state_t::running);
	if (!m_node) m_node = m_parent->m_pool.malloc();
	size_t size = 0;
	char* p;
	do {
		p = m_node->write(&size);
		size = size > len ? len : size;
		memcpy(p, src, size);
		m_node->commit_write(size);
	} while (len -= size);
}

void recorder::start(const char* filename, const char* ext, size_t filemax) {
	state_t exp = state_t::none;
	if (!m_state.compare_exchange_strong(exp, state_t::running))
		Clog::error_throw(errors::logic, "logsystem already running!");

	m_ext = ext;
	m_filename = filename;
	max_size = filemax;
}

void recorder::stop(void) {
	state_t exp = state_t::running;
	if (!m_state.compare_exchange_strong(exp, state_t::stopping))
		return;

	m_worker.safe_stop();
	if (m_file.is_open())
		m_file.close();

	m_state = state_t::none;
}

void recorder::write(const char* src, size_t len) {
	node_t* node = m_pool.malloc();
	size_t size = 0;
	char* p;
	do {
		p = node->write(&size);
		size = size > len ? len : size;
		memcpy(p, src, size);
		node->commit_write(size);
	} while (len -= size);
	m_worker.schedule(task_info{ this,node });
}

bool recorder::open_file(void) {
	char buffer[20];
	tm tmNow;
	tmNow.set();
	snprintf(buffer, 20, "_%04d%02d%02d%02d%02d%02d."
		, tmNow.tm_year + 1900, tmNow.tm_mon + 1, tmNow.tm_mday
		, tmNow.tm_hour, tmNow.tm_min, tmNow.tm_sec);

	std::string file = m_filename;
	file += buffer;
	file += m_ext;

	if (m_file.is_open())
		m_file.close();

	m_file.open(file, std::ios::out | std::ios::app | std::ios::binary);
	if (!m_file.is_open()) {
		Clog::error("recorder open file fail! (%s)", file.c_str());
		return false;
	}

	return true;
}

void recorder::save(node_t* p_recorder) {
	std::size_t size;
	const char* p;

	do {
		if (!m_file.is_open() && !open_file())
			break;

		p = p_recorder->read(size);
		if (0 == size)
			break;

		m_file.write(p, size);
		m_len += size;
		p_recorder->commit_read();

		if (!m_file.good())	break; 		// Ó²ÅÌÂúÁË
	} while (true);
	m_pool.free(p_recorder);
	if (m_len >= max_size) {
		m_file.close();
		m_len = 0;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace mem
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility