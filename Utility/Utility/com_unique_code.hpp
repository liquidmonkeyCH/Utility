/**
* @file com_unique_code.hpp
*
* @author Hourui (liquidmonkey)
**/

#ifndef __COM_UNIQUE_CODE_HPP__
#define __COM_UNIQUE_CODE_HPP__

#include <time.h>

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////
namespace com
{
////////////////////////////////////////////////////////////////////////////////
class unique_code
{
public:
	unique_code(std::uint16_t key) : m_time(time(nullptr)), m_index(0), m_key(key) { m_key <<= 16; }
	~unique_code(void) = default;

	std::uint64_t gen(void) {
		std::uint64_t code = m_time;
		code = code << 32;
		code |= m_key;
		code |= m_index;
		if (++m_index > 0xFFFF)
		{
			m_index = 0;
			++m_time;
		}
		return code;
	}
	time_t get_seed(void) { return m_time; }
private:
	time_t m_time;
	std::uint32_t m_index;
	std::uint32_t m_key;
};
////////////////////////////////////////////////////////////////////////////////
}// namespace com 
////////////////////////////////////////////////////////////////////////////////
}// namespace Utility
#endif //__COM_UNIQUE_CODE_HPP__
