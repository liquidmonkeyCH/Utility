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
#include <map>
#include <vector>
#include <atomic>

#include "com_less.hpp"

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
	using param_list = std::vector<std::string>;
	using param_map = std::map<const char*,int,com::strless>;
	friend void on_signal(int n);
public:
	bool Start(int param_num, char* params[]);
	void Run(void);

	void yeild(void);
	void resume(void);
protected:
	size_t get_param_num(void);
	const char* get_param(size_t n);
protected:
	virtual bool OnStart(void) = 0;
	virtual void OnStop(void) = 0;
private:
	void daemon(void);
	static void setsignal(void);
	static controler* get_controler(void);
private:
	param_list				m_param_list;
};
////////////////////////////////////////////////////////////////////////////////
}
////////////////////////////////////////////////////////////////////////////////
} //namespace Utility
#endif //__APPLICATION_HPP__

