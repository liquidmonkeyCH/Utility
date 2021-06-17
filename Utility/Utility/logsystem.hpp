/**
* @file logsystem.hpp
*
* @author Hourui (liquidmonkey)
**/

#ifndef __UTILITY_LOG_SYSTEM_HPP__
#define __UTILITY_LOG_SYSTEM_HPP__

#include "mem_recorder.hpp"
#include "com_singleton.hpp"
#include "com_thread_pool.hpp"
#include "com_time.hpp"
#include "logger.hpp"

#include <fstream>

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
	static constexpr size_t HEAD_LEN = 30;
	static constexpr size_t MAX_LEN = Clog::MAX_LOG_LEN + HEAD_LEN;
	static constexpr time_t DAY = 24 * 3600;
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
	~logsystem(void) { stop(); }

	logsystem(const logsystem&) = delete;
	logsystem& operator=(const logsystem&) = delete;

	void start(const char* filename = "./log", std::uint8_t lv = logsystem::level::info, size_t max = logsystem::max_file_size, std::uint8_t interval = 24);
	void stop(void);
private:
	bool open_file(void);
	void save(recorder* p_recorder);
	void log_out(const char* str, std::uint8_t lv);

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
	time_t m_tm_tomorrow = 0;
	time_t m_tm_next = 0;
	time_t m_interval = 0;
	std::hash<std::thread::id> m_hash;

	std::string m_filename;
	std::ofstream m_file;
};
////////////////////////////////////////////////////////////////////////////////
}
////////////////////////////////////////////////////////////////////////////////
} //namespace Utility
#endif //__UTILITY_LOG_SYSTEM_HPP__

