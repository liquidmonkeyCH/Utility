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
	inline void set(time_t tm = time(nullptr)) { localtime_r(&tm, this); }
};

}// namespace com 
////////////////////////////////////////////////////////////////////////////////
}// namespace Utility
#endif //__COM_WIDGETS_HPP__
