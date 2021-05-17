/**
* @file com_unique_code.hpp
*
* @author Hourui (liquidmonkey)
**/

#ifndef __COM_UNIQUE_CODE_HPP__
#define __COM_UNIQUE_CODE_HPP__

#include <time.h>
#include <cstdint>
#include <mutex>

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////
namespace com
{
////////////////////////////////////////////////////////////////////////////////
class unique_code
{
public:
	unique_code(void) = default;
	unique_code(std::uint16_t key) : m_time(time(nullptr)), m_index(0), m_key(key) { m_key <<= 16; }
	~unique_code(void) = default;

	std::uint64_t gen(void) {
		std::lock_guard<std::mutex> lock(m_mutex);
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
	inline time_t get_seed(void) { std::lock_guard<std::mutex> lock(m_mutex); return m_time; }
	inline time_t set_seed(time_t tm) { std::lock_guard<std::mutex> lock(m_mutex); m_time = tm; }
	inline time_t set_key(std::uint16_t key) { std::lock_guard<std::mutex> lock(m_mutex); m_key = key; m_key <<= 16; }
private:
	std::mutex m_mutex;
	time_t m_time;
	std::uint32_t m_index;
	std::uint32_t m_key;
};
////////////////////////////////////////////////////////////////////////////////
}// namespace com 
////////////////////////////////////////////////////////////////////////////////
}// namespace Utility
#endif //__COM_UNIQUE_CODE_HPP__
