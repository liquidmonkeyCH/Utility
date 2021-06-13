/**
* @file com_recorder.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __COM_RECORDER_HPP__
#define __COM_RECORDER_HPP__

#include "mem_recorder.hpp"
#include "com_time.hpp"
#include "logger.hpp"
#include <fstream>

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace com
{
////////////////////////////////////////////////////////////////////////////////////////////////////
template<std::size_t block_size>
class recorder
{
private:
	enum class state_t { none, running, stopping };
	using node_t = mem::recorder<block_size>;
	struct task_info
	{
		recorder* m_recorder;
		node_t* m_node;
		void exec(void) { m_recorder->save(m_node); }
	};
public:
	recorder(void) = default;
	~recorder(void) { stop(); }

	void start(const char* filename, const char* ext, size_t filemax) {
		state_t exp = state_t::none;
		if (!m_state.compare_exchange_strong(exp, state_t::running))
			Clog::error_throw(errors::logic, "logsystem already running!");

		m_ext = ext;
		m_filename = filename;
		max_size = filemax;
	}
	void stop(void) {
		state_t exp = state_t::running;
		if (!m_state.compare_exchange_strong(exp, state_t::stopping))
			return;

		m_worker.safe_stop();
		if (m_file.is_open())
			m_file.close();

		m_state = state_t::none;
	}

	void insert(const char* src,size_t len) {
		node_t* node = m_pool.malloc();
		size_t size = 0;
		char* p;
		do {
			p = node->write(&size);
			size = size > len ? len : size;
			memcpy(p, src, size);
			if (node->commit_write(size))
				m_worker.schedule(task_info{ this,node });
		} while (len -= size);
		m_pool.free(node);
	}
private:
	bool open_file(void) {
		char buffer[20];
		com::tm tmNow;
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

	void save(recorder * p_recorder) {
		std::size_t size;
		const char* p;

		do {
			if (!m_file.good())
				if (!open_file())
					return;
					
			p = p_recorder->read(size);
			if (0 == size)
				return;

			m_file.write(p, size);
			m_len += size;
			p_recorder->commit_read();

			if(m_len > max_size)
				if (!open_file())
					return;
		} while (true);
	}
private:
	com::task_thread<task_info> m_worker;
	mem::data_pool_s<node_t, 5> m_pool;
	std::atomic<state_t> m_state = state_t::none;
	size_t max_size = 0;
	size_t m_len = 0;

	std::string m_ext;
	std::string m_filename;
	std::ofstream m_file;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace mem
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__COM_RECORDER_HPP__