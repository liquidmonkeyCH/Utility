/**
* @file com_recorder.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __COM_RECORDER_HPP__
#define __COM_RECORDER_HPP__

#include "mem_recorder.hpp"
#include "com_time.hpp"
#include "com_thread_pool.hpp"
#include "logger.hpp"
#include <fstream>

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace com
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class recorder
{
private:
	enum class state_t { none, running, stopping };
	using node_t = mem::recorder<512>;
	struct task_info
	{
		recorder* m_recorder;
		node_t* m_node;
		void exec(void) { m_recorder->save(m_node); }
	};
	static constexpr size_t file_max = 1024 * 1024 * 1024;
public:
	class noter 
	{
	public:
		noter(void) = delete;
		~noter(void) = default;

		noter(noter&&) = default;
		noter& operator=(noter&&) noexcept = default;

		noter(const noter&) = delete;
		noter& operator=(const noter&) = delete;

		void write(const char* src, size_t len);
		void commit(void) { m_parent->m_worker.schedule(task_info{ m_parent,m_node }); m_node = nullptr; }
	private:
		friend class recorder;
		noter(recorder* p) : m_parent(p){}
	private:
		recorder* m_parent = nullptr;
		node_t* m_node = nullptr;
	};
	friend class noter;
public:
	recorder(void) = default;
	~recorder(void) { stop(); }

	recorder(const recorder&) = delete;
	recorder& operator=(const recorder&) = delete;

	void start(const char* filename, const char* ext, size_t filemax = file_max);
	void stop(void);

	void write(const char* src, size_t len);
	inline noter&& get_noter(void) { return std::move(noter{ this }); }
private:
	bool open_file(void);
	void save(node_t* p_recorder);
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

template<size_t block_size>
class recorder_f
{
private:
	enum class state_t { none, running, stopping };
	using node_t = mem::recorder<block_size>;
	struct task_info
	{
		recorder_f* m_recorder;
		node_t* m_node;
		void exec(void) { m_recorder->save(m_node); }
	};
	static constexpr size_t file_max = 1024 * 1024 * 1024;
public:
	recorder_f(void) = default;
	~recorder_f(void) { stop(); }

	recorder_f(const recorder_f&) = delete;
	recorder_f& operator=(const recorder_f&) = delete;

	void start(const char* filename, const char* ext, size_t filemax = file_max) {
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

	// 一次需要写一整条(不保证两次写入的顺序)
	void write(const char* src, size_t len) {
		assert(len <= block_size);
		node_t* node = m_pool.malloc();
		char* p = node->write();
		memcpy(p, src, len);
		if(node->commit_write(len))
			m_worker.schedule(task_info{ this,node });
		m_pool.free(node);
	}
private:
	bool open_file(void) {
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
	void save(node_t * p_recorder) {
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

			if (!m_file.good())	break; 		// 硬盘满了
			if (m_len >= max_size) {
				m_file.close();
				m_len = 0;
			}
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