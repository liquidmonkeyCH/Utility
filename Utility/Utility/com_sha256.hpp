/**
* @file com_sha256.hpp
*
* @author Hourui (liquidmonkey)
**/

#ifndef __COM_SHA256_HPP__
#define __COM_SHA256_HPP__

#include "Utility/com_hex_caster.hpp"
#include <cstdio>

namespace Utility
{
//////////////////////////////////////	//////////////////////////////////////////
namespace com
{
////////////////////////////////////////////////////////////////////////////////
class sha256
{
	static constexpr std::uint32_t init_data[] = {
		0x6a09e667, 0xbb67ae85, 0x3c6ef372,
		0xa54ff53a, 0x510e527f, 0x9b05688c,
		0x1f83d9ab, 0x5be0cd19
	};
	static constexpr std::uint32_t TAIL_SIZE = 128;
	static constexpr std::uint32_t BLOCK_SIZE = 64;
	static constexpr std::uint32_t SIZE_SIZE = 8;
	static constexpr std::uint32_t DEAD_LINE = BLOCK_SIZE - 9;
public:
	sha256(void);
	sha256(const void* data, std::size_t len);

	sha256(const sha256&) = delete;
	sha256& operator=(const sha256&) = delete;

	void reset(void);
	void update(const void* input, std::size_t length);

	const char* c_str(void) { digest(); return m_result; }
	const uint32_t* gen_out(void) { digest(); return m_digest; }
private:
	void final(void);
	void transform(const std::uint32_t* chunk, std::size_t blocks);
	void digest(void) {
		if (m_finished) return;
		m_finished = true;
		final();
		sprintf_s(m_result, "%08x%08x%08x%08x%08x%08x%08x%08x", 
			m_digest[0], m_digest[1], m_digest[2], m_digest[3], m_digest[4], m_digest[5], m_digest[6], m_digest[7]);
	}
private:
	bool m_finished = false;
	const char* m_data = nullptr;

	std::size_t m_pos = 0;
	char m_tail[TAIL_SIZE];
	std::size_t m_size = 0;
	std::uint32_t m_state[8];

	std::uint32_t m_digest[8];
	char m_result[64 + 1] = {};
};
////////////////////////////////////////////////////////////////////////////////
}// namespace com
////////////////////////////////////////////////////////////////////////////////
}// namespace Utility
#endif // __COM_SHA256_HPP__