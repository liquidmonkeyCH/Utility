// MIT License
//
// Copyright (c) 2016-2017 Simon Ninon <simon.ninon@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

/**
* @file logger.hpp
*
* @author Hourui (liquidmonkey)
*/

#ifndef __LOGGER_HPP__
#define __LOGGER_HPP__

#include <stdarg.h>
#include <cstdint>
#include <mutex>
#include <stdexcept>
#include <thread>
#include "com_time.hpp"
#include "com_hex_caster.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
enum class errors {
	none,
	system,
	logic,
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class utility_error : public std::runtime_error
{
public:
	utility_error(const std::int64_t error_no, const char* what = nullptr) : std::runtime_error(what ? what : ""), m_error(error_no) {}
	~utility_error(void) = default;

	utility_error(const utility_error&) = default;
	utility_error& operator=(const utility_error&) = default;
public:
	std::int64_t get_error(void) const { return m_error; }
private:
	std::int64_t m_error;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
enum class log_type { error, warn, info, echo, dump, debug };
static constexpr const char* log_title[] = { "ERRO","WARN","INFO","ECHO","DUMP","DBUG" };
////////////////////////////////////////////////////////////////////////////////////////////////////
class logger_iface 
{
public:
	logger_iface(void) = default;
	virtual ~logger_iface(void) = default;
	logger_iface(const logger_iface&) = delete;
	logger_iface& operator=(const logger_iface&) = delete;

	friend class Clog;
private:
	virtual void output(log_type t, const char* head, const char* str) = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class logger : public logger_iface 
{
public:
	logger(void) = default;
	logger(const logger&) = delete;
	logger& operator=(const logger&) = delete;
private:
	void output(log_type t, const char* head, const char* str);
private:
	std::mutex m_mutex;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
#define LOG_HEAD()													\
	char head[HEAD_LEN];											\
	com::tm tmNow;													\
	tmNow.set();													\
	snprintf(head, HEAD_LEN											\
	, "[%02d:%02d:%02d][%016llX]"									\
	, tmNow.tm_hour, tmNow.tm_min, tmNow.tm_sec						\
	, (long long)m_hash(std::this_thread::get_id()));
#define LOG_FORMAT(fmt)												\
	char str[MAX_LOG_LEN+2];										\
	va_list arglist;												\
	va_start(arglist, fmt);											\
	vsnprintf(str, MAX_LOG_LEN, fmt, arglist);						\
	va_end(arglist);												\
	std::size_t len = strlen(str);									\
	str[len] = 0x0;
#define LOG_OUT(type,fmt)											\
	if(m_logger){													\
	LOG_HEAD()														\
	LOG_FORMAT(fmt)													\
	m_logger->output(type,head,str);}
#define LOG_THROW(e,type,fmt)										\
	LOG_FORMAT(fmt)													\
	if(m_logger){ LOG_HEAD() m_logger->output(type,head,str); }		\
	throw utility_error(static_cast<std::int64_t>(e),str);	
#define LOG_ECHO(type,data){										\
	if(!m_logger) return;											\
	LOG_HEAD()														\
	m_logger->output(type,head,data);}
////////////////////////////////////////////////////////////////////////////////////////////////////
class Clog
{
public:
	static const int MAX_LOG_LEN = 1024;
	static constexpr size_t HEAD_LEN = 35;
	static constexpr size_t MAX_LEN = MAX_LOG_LEN + HEAD_LEN;
	static constexpr size_t MAX_SIZE = MAX_LEN+2;

	struct level {
		static constexpr std::uint8_t normal = 0;
		static constexpr std::uint8_t debug = 99;
	};

	Clog(void) = delete;
	Clog(const Clog&) = delete;
	Clog& operator=(const Clog&) = delete;
	Clog(const Clog&&) = delete;
	Clog& operator=(const Clog&&) = delete;
public:	
	static void active_logger(logger_iface* _logger)			{ m_logger = _logger; }
	static void set_level(std::uint8_t lv)						{ m_level = lv; }
	static void set_warn_lv(std::uint8_t lv)					{ m_warn = lv; }

	template<class T>
	static void error_throw(T e_no, const char* fmt, ...)		{ LOG_THROW(e_no, log_type::error, fmt) }
	static void error(const char* fmt, ...)						{ LOG_OUT(log_type::error, fmt) }
	static void warn(const char* fmt, ...)						{ LOG_OUT(log_type::warn, fmt) }
	static void warn(std::uint8_t lv, const char* fmt, ...)		{ if (m_warn >= lv) LOG_OUT(log_type::warn, fmt) }
	static void info(const char* fmt, ...)						{ LOG_OUT(log_type::info, fmt) }
	static void info(std::uint8_t lv, const char* fmt, ...)		{ if (m_level >= lv) LOG_OUT(log_type::info, fmt) }
	static void echo(const char* str)							{ LOG_ECHO(log_type::echo, str) }
	static void echo(std::uint8_t lv, const char* str)			{ if (m_level >= lv) LOG_ECHO(log_type::echo, str) }
	static void debug(const char* fmt, ...)						{ if(m_level >= level::debug) LOG_OUT(log_type::debug, fmt) }
	
	template<class T>
	static void dump(std::uint8_t lv, const T& data)			{ if(m_level >= lv) LOG_ECHO(log_type::dump, com::bin2hex(data).str()) }
	template<class T> static void dump(const T& data)			{ LOG_ECHO(log_type::dump, com::bin2hex(data).str()) }
private:
	static logger_iface* m_logger;
	static std::uint8_t m_level;
	static std::uint8_t m_warn;
	static std::hash<std::thread::id> m_hash;
};
#undef LOG_HEAD
#undef LOG_FORMAT
#undef LOG_OUT
#undef LOG_THROW
#undef LOG_ECHO
////////////////////////////////////////////////////////////////////////////////////////////////////
}// namespace Utility
#endif //__LOGGER_HPP__