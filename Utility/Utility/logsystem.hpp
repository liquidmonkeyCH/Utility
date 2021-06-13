/**
* @file logsystem.hpp
*
* @author Hourui (liquidmonkey)
**/

#ifndef __UTILITY_LOG_SYSTEM_HPP__
#define __UTILITY_LOG_SYSTEM_HPP__

#include "com_thread_pool.hpp"
#include "com_time.hpp"
#include "com_singleton.hpp"
#include "mem_recorder.hpp"
#include "logger.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////
namespace main
{
////////////////////////////////////////////////////////////////////////////////
class logsystem : public logger_iface, public com::iface::Singleton<logsystem>
{
public:
	struct level {
		static constexpr std::uint8_t error = 0;
		static constexpr std::uint8_t warn = 1;
		static constexpr std::uint8_t info = 2;
		static constexpr std::uint8_t debug = 3;
		static constexpr std::uint8_t other = 4;
	};
	static constexpr size_t max_file_size = 1024 * 1024 * 1024;
private:
	static constexpr size_t HEAD_LEN = 38;
	static constexpr size_t MAX_LEN = Clog::MAX_LOG_LEN + HEAD_LEN;
	enum class state_t { none, running, stopping };
	using recorder = mem::recorder<MAX_LEN>;
	struct task_info
	{
		logsystem* m_log;
		recorder* m_recorder;
		void exec(void) { m_log->save(m_recorder); }
	};
	static constexpr const char* out_type[] = { "ERRO","WARN","INFO","DBUG","LV%02X" };
public:
	logsystem(void) = default;
	~logsystem(void) { m_worker.safe_stop(); }

	void start(const char* filename = "log", std::uint8_t lv = logsystem::level::info, size_t max = logsystem::max_file_size){
		state_t exp = state_t::none;
		if (!m_state.compare_exchange_strong(exp, state_t::running))
			Clog::error_throw(errors::logic, "logsystem already running!");

		m_level = lv;
		max_size = max;
	}
	void stop(void) {
		state_t exp = state_t::running;
		if (!m_state.compare_exchange_strong(exp, state_t::stopping))
			return;

		m_worker.safe_stop();
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

			m_len += size;
			// log save
			std::cout << p;

			p_recorder->commit_read();
		} while (true);
	}

	inline void log_out(const char* str, std::uint8_t lv) {
		if(m_level < lv) return;
		recorder* rec = m_pool.malloc();
		char* buffer = rec->write();
		if (lv > level::debug)
			snprintf(buffer + 1, 10, out_type[level::other], lv);
		else
			memcpy(buffer + 1, out_type[lv], 5);

		tm tmNow;
		time_t tNow = time(nullptr);
		localtime_r(&tNow, &tmNow);
		snprintf(buffer, MAX_LEN
			, "[%s][%04d-%02d-%02d %02d:%02d:%02d][%08X] %s", buffer + 1
			, tmNow.tm_year + 1900, tmNow.tm_mon + 1, tmNow.tm_mday
			, tmNow.tm_hour, tmNow.tm_min, tmNow.tm_sec
			, ::GetCurrentThreadId(), str);

		std::size_t len = strlen(buffer);
		buffer[len] = 0x0;
		if(rec->commit_write(len))
			m_worker.schedule(task_info{this,rec});
		
		m_pool.free(rec);
	}

	inline void debug(const char* str) { log_out(str, 3); }
	inline void info(const char* str) { log_out(str, 2); }
	inline void warn(const char* str) { log_out(str, 1); }
	inline void error(const char* str) { log_out(str, 0); }
	inline void log(std::uint32_t lv, const char* str) { log_out(str, lv); }
private:
	std::mutex m_mutex;
	com::task_thread<task_info> m_worker;
	mem::data_pool_s<recorder, 5> m_pool;
	std::atomic<state_t> m_state = state_t::none;
	std::uint8_t m_level = 0;
	size_t max_size = 0;
	size_t m_len = 0;

	std::ofstream m_file;
};
////////////////////////////////////////////////////////////////////////////////
}
////////////////////////////////////////////////////////////////////////////////
} //namespace Utility
#endif //__UTILITY_LOG_SYSTEM_HPP__

