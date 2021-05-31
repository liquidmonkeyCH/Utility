/**
* @file com_sha256.hpp
*
* @author Hourui (liquidmonkey)
**/

#ifndef __COM_SHA256_HPP__
#define __COM_SHA256_HPP__

#include "Utility/com_hex_caster.hpp"

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
public:
	sha256(const char* data = nullptr);
	~sha256(void) = default;

	void reset(const char* data);
	const char* c_str(void) { return m_finished ? m_result : _impl::bin_to_hex<false>(m_result, digest(), 32); }
private:
	void init(void);
	void transform(const uint32_t* chunk, size_t blocks);
	const unsigned char* digest(void);
private:
	bool m_finished = false;
	const char* m_data = nullptr;
	size_t m_blocks = 0;
	std::uint32_t m_digest[8];
	char m_result[64 + 1] = {};
};
////////////////////////////////////////////////////////////////////////////////
}// namespace com
////////////////////////////////////////////////////////////////////////////////
}// namespace Utility
#endif // __COM_SHA256_HPP__