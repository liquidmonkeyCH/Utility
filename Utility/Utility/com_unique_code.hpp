/**
* @file com_unique_code.hpp
*
* @author Hourui (liquidmonkey)
**/

#ifndef __COM_UNIQUE_CODE_HPP__
#define __COM_UNIQUE_CODE_HPP__

#include <cstdint>
#include <atomic>
#include <queue>
#include "logger.hpp"
#include "com_time.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////
namespace com
{
////////////////////////////////////////////////////////////////////////////////
class unique_code
{
public:
	unique_code(std::uint16_t key = 0, std::uint32_t seed = 0xFFFFFFFF & time(nullptr)) { reset(key, seed); }
	~unique_code(void) = default;
	inline void reset(std::uint16_t key = 0, std::uint32_t seed = 0xFFFFFFFF & time(nullptr)) {
		std::uint64_t code = key; code <<= 32;
		code |= seed; code <<= 16;
		m_code = code;
	}
	inline std::uint64_t gen(void) { return ++m_code; }
protected:
	std::atomic_uint64_t m_code;
};
////////////////////////////////////////////////////////////////////////////////
class uid : public unique_code
{
public:
	uid(const char* Name = "", std::uint16_t key = 0, std::uint32_t seed = 0xFFFFFFFF & time(nullptr))
		: m_name(Name), unique_code(key, seed) {}
	~uid(void) {
		std::uint64_t code = m_code.load() >> 16;
		std::uint32_t seed = 0xFFFFFFFF & code;
		time_t now = time(nullptr);
		if (seed >= (0xFFFFFFFF & now)) {
			com::tm tm_seed;
			tm_seed.set((now | 0xFFFFFFFF) & seed);
			Clog::warn("uid(%s)[key=%lld] destroy seed[%04d-%02d-%02d %02d:%02d:%02d]", 
				m_name.c_str(), (code >> 32)
				, tm_seed.tm_year + 1900
				, tm_seed.tm_mon + 1
				, tm_seed.tm_mday
				, tm_seed.tm_hour
				, tm_seed.tm_min
				, tm_seed.tm_sec);
		}
			
	}
private:
	std::string m_name;
};
////////////////////////////////////////////////////////////////////////////////
}// namespace com 
////////////////////////////////////////////////////////////////////////////////
}// namespace Utility
#endif //__COM_UNIQUE_CODE_HPP__
