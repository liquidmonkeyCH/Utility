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
	static constexpr size_t max_file_size = 1024 * 1024 * 1024;
private:
	static constexpr time_t DAY = 24 * 3600;
	enum class state_t { none, running, stopping };
	using recorder = mem::recorder<Clog::MAX_SIZE>;
	struct task_info
	{
		logsystem* m_log;
		recorder* m_recorder;
		void exec(void) { m_log->save(m_recorder); }
	};
public:
	logsystem(void) = default;
	~logsystem(void) { stop(); }

	logsystem(const logsystem&) = delete;
	logsystem& operator=(const logsystem&) = delete;

	void start(const char* filename = "./log", size_t max = logsystem::max_file_size, std::uint8_t interval = 24);
	void stop(void);
private:
	bool open_file(void);
	void save(recorder* p_recorder);
	void output(log_type t, const char* head, const char* str);
private:
	std::mutex m_mutex;
	com::task_thread<task_info> m_worker;
	mem::data_pool_s<recorder, 5> m_pool;
	std::atomic<state_t> m_state = state_t::none;
	size_t max_size = 0;
	size_t m_len = 0;
	time_t m_tm_tomorrow = 0;
	time_t m_tm_next = 0;
	time_t m_interval = 0;

	std::string m_filename;
	std::ofstream m_file;
};
////////////////////////////////////////////////////////////////////////////////
}
////////////////////////////////////////////////////////////////////////////////
} //namespace Utility
#endif //__UTILITY_LOG_SYSTEM_HPP__

