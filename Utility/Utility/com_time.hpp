/**
* @file com_time.hpp
*
* @author Hourui (liquidmonkey)
**/

#ifndef __COM_TIME_HPP__
#define __COM_TIME_HPP__

#include <time.h>
#include <type_traits>

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
	inline virtual void set(time_t t = time(nullptr)) { localtime_r(&t, this); }
	inline time_t get(void) { return mktime(this); }
};

struct time_delimiter
{
	static constexpr char empty = 0;
	static constexpr char minus = '-';
	static constexpr char slash = '/';
	static constexpr char colon = ':';
	static constexpr char space = ' ';
};

class date : public tm, public time_delimiter
{
public:
	date(char delimiter = minus) : m_delimiter{ delimiter } {}
	~date(void) = default;

	inline void set(time_t t = time(nullptr)) { tm::set(t); m_str[0] = 0; }
	inline const char* to_str(void) {
		if (0 == *m_str)
			snprintf(m_str, 11, "%04d%s%02d%s%02d", 
				this->tm_year + 1900, m_delimiter, this->tm_mon + 1, m_delimiter, this->tm_mday);
		return m_str;
	}
private:
	char m_str[11] = {0};
	char m_delimiter[2] = {0};
};

class timestamp : public tm, public time_delimiter
{
public:
	timestamp(char delimiter = colon) : m_delimiter{ delimiter } {}
	~timestamp(void) = default;

	inline void set(time_t t = time(nullptr)) { tm::set(t); m_str[0] = 0; }
	inline const char* to_str(void) {
		if (0 == *m_str)
			snprintf(m_str, 10, "%02d%s%02d%s%02d",
				this->tm_hour, m_delimiter, this->tm_min, m_delimiter, this->tm_sec);
		return m_str;
	}
private:
	char m_str[10] = { 0 };
	char m_delimiter[2] = { 0 };
};

class date_time : public tm, public time_delimiter
{
public:
	date_time(char date_delimiter = minus, char time_delimiter = colon,char delimiter = space)
		: m_date_delimiter{ date_delimiter }, m_time_delimiter{ time_delimiter }, m_delimiter{delimiter} {}
	~date_time(void) = default;

	inline void set(time_t t = time(nullptr)) { tm::set(t); m_str[0] = 0; }
	inline const char* to_str(void) {
		if (0 == *m_str)
			snprintf(m_str, 20, "%04d%s%02d%s%02d%s%02d%s%02d%s%02d",
				this->tm_year + 1900, m_date_delimiter, this->tm_mon + 1, m_date_delimiter, this->tm_mday,
				m_delimiter,this->tm_hour, m_time_delimiter,this->tm_min, m_time_delimiter, this->tm_sec);
		return m_str;
	}
private:
	char m_str[20] = { 0 };
	char m_date_delimiter[2] = { 0 };
	char m_time_delimiter[2] = { 0 };
	char m_delimiter[2] = { 0 };
};

}// namespace com 
////////////////////////////////////////////////////////////////////////////////
}// namespace Utility
#endif //__COM_WIDGETS_HPP__
