/**
* @file application.hpp
*
* @author Hourui (liquidmonkey)
**/

#ifndef __APPLICATION_HPP__
#define __APPLICATION_HPP__

#include <mutex>
#include <condition_variable>
#include <string>
#include <set>
#include <vector>
#include <atomic>

#include "com_less.hpp"
#include "Utility/logger.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////
namespace main
{
////////////////////////////////////////////////////////////////////////////////
class application
{
private:
	class controler
	{
	public:
		controler(void) : m_state(state::none) {};
		~controler(void) = default;
	public:
		friend class application;
		void stop(void);
	private:
		bool start(void);
		void run(void);
		void yield(void);
		void resume(void);
	private:
		enum class state { none, running, stopping };
		std::atomic<state>	m_state;
		std::mutex m_mutex;
		std::condition_variable	m_cv;
	};
public:
	application(void) = default;
	~application(void) = default;
	using param_list = std::vector<const char*>;
	using param_set = std::set<const char*,com::strless>;
	friend void on_signal(int n);
public:
	bool Start(int param_num, char* params[]);
	void Run(void);

	inline bool is_daemon(void) { return m_daemon; }
	void yeild(void);
	void resume(void);
	inline const char* get_path(void) { return m_path.c_str(); }
	inline const char* get_name(void) { return m_name; }
	inline const char* get_full_name(void) { return m_param_list.front(); }
protected:
	size_t get_param_num(void);
	const char* get_param(size_t n);
	bool has_param(const char* param);
protected:
	virtual bool OnStart(void) = 0;
	virtual void OnStop(void) = 0;
private:
	void daemon(void);
	static void setsignal(void);
	static controler* get_controler(void);
private:
	param_list				m_param_list;
	param_set				m_param_set;
	const char*				m_name;
	std::string				m_path;
	bool					m_daemon = false;
	logger					m_logger{ logger::log_level::debug };
};
////////////////////////////////////////////////////////////////////////////////
}
////////////////////////////////////////////////////////////////////////////////
} //namespace Utility
#endif //__APPLICATION_HPP__

