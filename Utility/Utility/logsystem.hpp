/**
* @file logsystem.hpp
*
* @author Hourui (liquidmonkey)
**/

#ifndef __UTILITY_LOG_SYSTEM_HPP__
#define __UTILITY_LOG_SYSTEM_HPP__

#include "com_thread_pool.hpp"
#include "com_singleton.hpp"
#include "com_guard.hpp"
#include "mem_recorder.hpp"
#include "logger.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////
namespace main
{
////////////////////////////////////////////////////////////////////////////////
template<size_t block_size>
class logsystem : public logger_iface, public com::iface::Singleton<logsystem<block_size>>
{
public:
	enum class log_level {
		error = 0,
		warn = 1,
		info = 2,
		debug = 3
	};
	static constexpr size_t max_len = block_size;
	DECLARE_SINGLETON(logsystem)
private:
	enum class state_t { none, running, stopping };
	using recorder = mem::recorder<block_size>;
	struct task_info
	{
		logsystem* m_log;
		recorder* m_recorder;
		void exec(void) { m_log->save(m_recorder); }
	};
	struct node_t 
	{
		recorder m_current;
		node_t* m_next = nullptr;
	};
	static constexpr const char* out_type[] = { "ERRO","WARN","INFO","DBUG" };
public:
	void start(log_level lv,const char* filename,size_t max) {
		state_t exp = state_t::none;
		if (!m_state.compare_exchange_strong(exp, state_t::running))
			Clog::error_throw(errors::logic, "logsystem already running!");

		m_level = (int)lv;
		max_size = max;
	}
	void stop(void) {
		state_t exp = state_t::running;
		if (!m_state.compare_exchange_strong(exp, state_t::stopping))
			return;

		m_workers.safe_stop();
		m_state = state_t::none;
	}
private:
	void save(recorder* p_recorder) {
		std::size_t size;
		const char* p;
		do {
			p = p_recorder->read(size);
			if (0 == size)
				return;

			// log save
			std::cout << p;

			p_recorder->commit_read();
		} while (true);
	}

	inline void log_out(const char* str, int n) {
		if(m_level < n) return;
		node_t* node;
		std::unique_lock<std::mutex> lock(m_mutex);
		if (nullptr == m_recorders) {
			node = m_pool.malloc();
		}else{
			node = m_recorders;
			m_recorders = node->m_next;
		}
		lock.unlock();
		tm tmNow;
		time_t tNow = time(nullptr);
		localtime_r(&tNow, &tmNow);
		char* buffer = node->m_current.write();
		snprintf(buffer, max_len
			, "[%s][%04d-%02d-%02d %02d:%02d:%02d][%08lX]%s", out_type[n]
			, tmNow.tm_year + 1900, tmNow.tm_mon + 1, tmNow.tm_mday
			, tmNow.tm_hour, tmNow.tm_min, tmNow.tm_sec
			, ::GetCurrentThreadId(), str);
		std::size_t len = strlen(buffer);
		buffer[len] = 0x0;
		if(node->m_current.commit_write(len))
			m_workers.schedule(task_info{this,&node->m_current});
		lock.lock();
		node->m_next = m_recorders;
		m_recorders = node;
	}
public:
	inline void debug(const char* str) { log_out(str, 3); }
	inline void info(const char* str) { log_out(str, 2); }
	inline void warn(const char* str) { log_out(str, 1); }
	inline void error(const char* str) { log_out(str, 0); }
private:
	std::mutex m_mutex;
	com::task_thread<task_info> m_workers;
	mem::data_pool<node_t, 5> m_pool;
	std::atomic<state_t> m_state = state_t::none;
	int m_level = 0;
	node_t* m_recorders = nullptr;
	size_t max_size = 0;
	size_t m_len = 0;

	std::ofstream m_file;
};
////////////////////////////////////////////////////////////////////////////////
template<size_t block_size>
logsystem<block_size>::logsystem(void){}
template<size_t block_size>
logsystem<block_size>::~logsystem(void) { m_workers.safe_stop(); }
////////////////////////////////////////////////////////////////////////////////
}
////////////////////////////////////////////////////////////////////////////////
} //namespace Utility
#endif //__UTILITY_LOG_SYSTEM_HPP__

