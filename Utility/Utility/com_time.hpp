/**
* @file com_time.hpp
*
* @author Hourui (liquidmonkey)
**/

#ifndef __COM_TIME_HPP__
#define __COM_TIME_HPP__

#include <time.h>

#ifdef _WIN32
inline tm* localtime_r(const time_t* timep, struct tm* result)
{
	localtime_s(result, timep);
	return result;
}
#endif // _WIN32

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////
namespace com
{

struct tm : public ::tm
{
	static constexpr time_t SEC_PER_MIN = 60;
	static constexpr time_t SEC_PER_HOUR = SEC_PER_MIN * 60;
	static constexpr time_t SEC_PER_DAY = SEC_PER_HOUR * 24;
	static constexpr time_t SEC_PER_WEEK = SEC_PER_DAY * 7;

	inline time_t get(void) { return mktime(this); }
	inline void set(time_t t = time(nullptr)) { localtime_r(&t, this); }
	tm& operator=(const ::tm& _tm) { *(::tm*)this = _tm; return *this; }

	inline tm& trim(void) { set(get()); return *this; }
	inline tm& trim_min(void) { trim(); tm_sec = 0; return *this; }
	inline tm& trim_hour(void) { trim(); tm_sec = tm_min = 0; return *this; }
	inline tm& trim_day(void) { trim(); tm_sec = tm_min = tm_hour = 0; return *this; }
	inline tm& trim_mont(void) { trim(); tm_sec = tm_min = tm_hour = 0; tm_mday = 1; return *this; }
	inline tm& trim_year(void) { trim(); tm_sec = tm_min = tm_hour = 0; tm_mday = 1; tm_mon = 0; return *this; }
};

class datetime : protected tm 
{
public:
	datetime(time_t t = time(nullptr)) { set(t); }
	inline void set(time_t t = time(nullptr)) { localtime_r(&t, this); init(); }
	inline const char* s_year(bool intact = true) const { return intact ? m_buffer : m_buffer + 2; }	// years since 1900
	inline const char* s_mon(void) const { return m_buffer + 5; }	// months since January - [1, 12]
	inline const char* s_day(void) const { return m_buffer + 8; }	// day of the month - [1, 31]
	inline const char* s_hour(void) const { return m_buffer + 11; }	// hours since midnight - [0, 23]
	inline const char* s_min(void) const { return m_buffer + 14; }	// minutes after the hour - [0, 59]
	inline const char* s_sec(void) const { return m_buffer + 17; }	// seconds after the minute - [0, 60] including leap second
	inline const char* c_str(void) const { return m_buffer + 20; }
	inline const char* to_str(char* buffer, size_t len, const char* fmt = "YYYY-MM-DD hh:mm:ss") const {
		memset(buffer, 0, len);
		size_t size = strlen(fmt);
		const char* p = buffer,*src;
		if (size > --len) size = len;
		for (size_t i = 0; i < size; i += len, fmt += len, buffer += len) {
			src = next(fmt, len);
			memcpy(buffer, src, len);
		}
		return p;
	}
	inline std::string to_str(const char* fmt = "YYYY-MM-DD hh:mm:ss") const {
		std::string ret;
		const char* src;
		size_t len,size = strlen(fmt);
		for (size_t i = 0; i < size; i += len, fmt += len) {
			src = next(fmt, len);
			ret.append(src, len);
		}
		return std::move(ret);
	}
private:
	inline void init(void) {
		snprintf(m_buffer, 20, "%04d-%02d-%02d %02d:%02d:%02d"
			, tm_year + 1900, tm_mon + 1,tm_mday
			, tm_hour, tm_min, tm_sec);
		memcpy(m_buffer + 20, m_buffer, 20);
		m_buffer[4] = m_buffer[7] = m_buffer[10] = m_buffer[13] = m_buffer[16] = 0;
	}

	inline const char* next(const char* fmt,size_t& len) const {
		if (fmt[0] < 0) { len = 2; return fmt; }
		if (fmt[0] != fmt[1]) { len = 1; return fmt; }
		len = 2;
		switch (fmt[0]) {
		case'Y':
			if (fmt[2] == fmt[3] && fmt[2] == fmt[0]) { len = 4; return m_buffer; }
			return m_buffer + 2;
		case'M': return s_mon();
		case'D': return s_day();
		case'h': return s_hour();
		case'm': return s_min();
		case's': return s_sec();
		default: break;
		}
		return fmt;
	}
private:
	char m_buffer[40] {0};
};

}// namespace com 
////////////////////////////////////////////////////////////////////////////////
}// namespace Utility
#endif //__COM_WIDGETS_HPP__
