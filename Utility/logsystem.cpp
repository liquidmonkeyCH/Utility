/**
* @file logsystem.cpp
*
* @author Hourui (liquidmonkey)
**/

#include "Utility/logsystem.hpp"
#ifdef _WIN32
#include <Windows.h>
#endif // _WIN32


#include <fstream>
#include <iostream>

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////
namespace main
{
////////////////////////////////////////////////////////////////////////////////
void logsystem::start(const char* filename, std::uint8_t lv, size_t max, std::uint8_t interval){
	state_t exp = state_t::none;
	if (!m_state.compare_exchange_strong(exp, state_t::running))
		Clog::error_throw(errors::logic, "logsystem already running!");

	m_filename = filename;
	m_level = lv;
	max_size = max;
	m_interval = time_t(interval) * 3600;
	time_t now = time(nullptr);
	com::tm tmNow;
	tmNow.set(now);
	tmNow.tm_min = 0;
	tmNow.tm_sec = 0;
	tmNow.tm_hour = 0;
	m_tm_tomorrow = tmNow.get() + DAY;
	m_tm_next = tmNow.get() + m_interval;
	while (m_tm_next < now) m_tm_next += m_interval;
	if (m_tm_next >= m_tm_tomorrow) {
		m_tm_next = m_tm_tomorrow;
		m_tm_tomorrow += DAY;
	}
}

void logsystem::stop(void) {
	state_t exp = state_t::running;
	if (!m_state.compare_exchange_strong(exp, state_t::stopping))
		return;

	m_worker.safe_stop();
	if (m_file.is_open())
		m_file.close();

	m_state = state_t::none;
}

bool logsystem::open_file(void) {
	char buffer[24];
	com::tm tmNow;
	tmNow.set();
	snprintf(buffer, 24, "_%04d%02d%02d%02d%02d%02d.log"
		, tmNow.tm_year + 1900, tmNow.tm_mon + 1, tmNow.tm_mday
		, tmNow.tm_hour, tmNow.tm_min, tmNow.tm_sec);

	std::string file = m_filename;
	file += buffer;

	if (m_file.is_open())
		m_file.close();
		
	m_file.open(file, std::ios::out | std::ios::app | std::ios::binary);
	if (!m_file.is_open()) {
		std::cout << "recorder open file fail!(" << file.c_str() << ")" << std::endl;
		return false;
	}

	return true;
}

void logsystem::save(recorder* p_recorder) {
	std::size_t size;
	const char* p;

	time_t now = time(nullptr);
	if (m_tm_next < now) {
		m_tm_next += m_interval;
		if (m_tm_tomorrow <= m_tm_next) {
			m_tm_next = m_tm_tomorrow;
			m_tm_tomorrow += DAY;
		}

		if (!open_file()) return;
	}

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
		if (m_len > max_size) {
			m_file.close();
			m_len = 0;
		}
	} while (true);

	if (m_file.is_open() && m_file.good())
		m_file.flush();

	m_pool.free(p_recorder);
}

void logsystem::log_out(const char* str, std::uint8_t lv) {
	if(m_level < lv) return;
	recorder* rec = m_pool.malloc();
	char* buffer = rec->write();
	if (lv > level::debug)
		snprintf(buffer + 1, 10, out_type[level::other], lv);
	else
		memcpy(buffer + 1, out_type[lv], 5);

	com::tm tmNow;
	tmNow.set();
	snprintf(buffer, MAX_LEN
		, "[%s][%02d:%02d:%02d][%016llX] %s\n", buffer + 1
		, tmNow.tm_hour, tmNow.tm_min, tmNow.tm_sec
		, (long long)m_hash(std::this_thread::get_id()), str);

	std::size_t len = strlen(buffer);
	buffer[len] = 0x0;
	rec->commit_write(len);
	m_worker.schedule(task_info{this,rec});
}
////////////////////////////////////////////////////////////////////////////////
}
////////////////////////////////////////////////////////////////////////////////
} //namespace Utility

