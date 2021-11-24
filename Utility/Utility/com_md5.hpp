/**
* @file com_md5.hpp
*
* @author Hourui (liquidmonkey)
**/

#ifndef __COM_MD5_HPP__
#define __COM_MD5_HPP__

#include "Utility/com_hex_caster.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////
namespace com
{
////////////////////////////////////////////////////////////////////////////////
class md5
{
public:
	md5();
	md5(const void* data, std::size_t len);

	md5(const md5&) = delete;
	md5& operator=(const md5&) = delete;

	void reset();
	void update(const void* input, std::size_t length);

	const char* c_str(void) { digest(); return _result; }
	const char* gen_out(void) { digest(); return (char*)_digest; }
private:
	void final();
	void transform(const unsigned char block[64]);
	void encode(const std::uint32_t* input, unsigned char* output, std::size_t length);
	void decode(const unsigned char* input, std::uint32_t* output, std::size_t length);

	void digest(void) {
		if (_finished) return;
		_finished = true;
		final();
		bin2hex<false>(_digest, _result, 33);
	}
private:
	char _result[32+1];
	std::uint32_t _state[4]; /* state (ABCD) */
	std::uint32_t _count[2]; /* number of bits, modulo 2^64 (low-order word first) */
	unsigned char _buffer[64]; /* input buffer */
	unsigned char _digest[16]; /* message digest */
	bool _finished; /* calculate finished ? */
	static constexpr unsigned char PADDING[64] = { 0x80 }; /* padding for calculate */
};
////////////////////////////////////////////////////////////////////////////////
}// namespace com
////////////////////////////////////////////////////////////////////////////////
}// namespace Utility
#endif // __COM_MD5_HPP__