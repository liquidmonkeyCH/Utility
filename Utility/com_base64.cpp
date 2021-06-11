/**
* @file com_base64.cpp
*
* @author Hourui (liquidmonkey)
**/

#include "Utility/com_base64.hpp"
#include <cstring>

namespace Utility
{
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace com
{
///////////////////////////////////////////////////////////////////////////////////////////////////
base64::base64(const char sign[2], char padding):padding_char(padding){
	if (sign) {
		chara[62] = sign[0];
		chara[63] = sign[1];
	}
	else
		chara[63] = '/';
}
///////////////////////////////////////////////////////////////////////////////////////////////////
size_t base64::encoding(const char* src, size_t src_size, char* dst, size_t dst_size, bool padding) {
	unsigned char pos = 0; // 索引是8位，但是高两位都为0
	pos = src_size % 3;
	if (pos == 0) {
		if (src_size / 3 * 4 > dst_size)
			return -1;
	}
	else {
		pos = padding ? 4 : pos + 1;
		if (src_size / 3 * 4 + pos > dst_size)
			return -1;
	}

	memset(dst, 0, dst_size);
	
	size_t i = 0, j = 0;
	while(i < src_size) {
		// 每三个一组，进行编码
		// 要编码的数字的第一个
		pos = ((src[i] >> 2) & 0x3f);
		dst[j++] = chara[pos];
		// 第二个
		pos = ((src[i] << 4) & 0x30);
		if (++i < src_size) {
			pos |= ((src[i] >> 4) & 0x0f);
			dst[j++] = chara[pos];
		}
		else {
			dst[j++] = chara[pos];
			if (padding) {
				dst[j++] = padding_char;
				dst[j++] = padding_char;
			}
			break; // 超出总长度，可以直接break
		}
		// 第三个
		pos = ((src[i] << 2) & 0x3c);
		if (++i < src_size) { // 有的话需要编码2个
			pos |= ((src[i] >> 6) & 0x03);
			dst[j++] = chara[pos];

			pos = src[i] & 0x3f;
			dst[j++] = chara[pos];
		}
		else {
			dst[j++] = chara[pos];
			if (padding) dst[j++] = padding_char;
			break;
		}

		++i;
	}

	return j;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
size_t base64::decoding(const char* src, size_t src_size, char* dst, size_t dst_size) {
	int x, y;
	x = (src_size & 0x3);
	if (x > 0 && x < 2) return -1;
	if ((src_size >> 2) * 3 + x - 1 > dst_size)
		return -1;

	memset(dst, 0, dst_size);
	
	size_t i = 0, j = 0;
	while (i < src_size && src[i] != padding_char) {
		x = find(src[i++]);
		if (x < 0) 
			return -1;

		y = find(src[i++]);
		if (y < 0)
			return -1;

		dst[j++] = ((x << 2) & 0xfc) | ((y >> 4) & 0x03);
		if (i >= src_size || src[i] == padding_char)
			return (y & 0x0f) ? -1 : j;
			
		x = find(src[i++]);
		if (x < 0)
			return -1;
			
		dst[j++] = ((x >> 2) & 0x0f) | ((y <<4) & 0xf0);
		if (i >= src_size || src[i] == padding_char)
			return (x & 0x03) ? -1 : j;

		y = find(src[i++]);
		if (y < 0)
			return -1;

		dst[j++] = ((x << 6) & 0xc0) | y;
	}

	return j;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
}// namespace com
///////////////////////////////////////////////////////////////////////////////////////////////////
}// namespace Utility