/**
* @file com_base64.hpp
*
* @author Hourui (liquidmonkey)
**/

#ifndef __COM_BASE64_HPP__
#define __COM_BASE64_HPP__

#include <string>

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////
namespace com
{
////////////////////////////////////////////////////////////////////////////////
class base64
{
private:
	char chara[64] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+";
	char padding_char;

	inline static int find_chara(char ch) {
		int pos = ch - 'A';
		if (pos >= 0 && pos < 26) return pos;
		pos = ch - 'a';
		if (pos >= 0 && pos < 26) return pos + 26;
		pos = ch - '0';
		if (pos >= 0 && pos < 10) return 52 + pos;
		return -1;
	}
	inline int find(char ch) {
		int pos = find_chara(ch);
		return pos < 0 ? (chara[62] == ch ? 62 : (chara[63] == ch ? 63 : -1)) : pos;
	}
public:
	base64(const char sign[2] = nullptr,char padding = '=');
	~base64(void) = default;

	size_t encoding(const char* src, char* dst, size_t size, bool padding = true);
	size_t decoding(const char* src, char* dst, size_t size);
};


////////////////////////////////////////////////////////////////////////////////
}// namespace com 
////////////////////////////////////////////////////////////////////////////////
}// namespace Utility
#endif //__COM_BASE64_HPP__
