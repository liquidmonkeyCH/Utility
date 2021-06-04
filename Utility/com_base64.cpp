/**
* @file com_base64.cpp
*
* @author Hourui (liquidmonkey)
**/

#include "Utility/com_base64.hpp"

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
size_t base64::encoding(const char* src, char* dst, size_t size, bool padding) {
	size_t len = strlen(src);
	if (len == -1) return -1;
	unsigned char pos = 0; // ������8λ�����Ǹ���λ��Ϊ0
	pos = len % 3;
	if (pos == 0) {
		if (len / 3 * 4 > size)
			return -1;
	}
	else {
		pos = padding ? 4 : pos + 1;
		if (len / 3 * 4 + pos > size)
			return -1;
	}

	memset(dst, 0, size);
	
	int i = 0, j = 0;
	while(i < len) {
		// ÿ����һ�飬���б���
		// Ҫ��������ֵĵ�һ��
		pos = ((src[i] >> 2) & 0x3f);
		dst[j++] = chara[pos];
		// �ڶ���
		pos = ((src[i] << 4) & 0x30);
		if (++i < len) {
			pos |= ((src[i] >> 4) & 0x0f);
			dst[j++] = chara[pos];
		}
		else {
			dst[j++] = chara[pos];
			if (padding) {
				dst[j++] = padding_char;
				dst[j++] = padding_char;
			}
			break; // �����ܳ��ȣ�����ֱ��break
		}
		// ������
		pos = ((src[i] << 2) & 0x3c);
		if (++i < len) { // �еĻ���Ҫ����2��
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
size_t base64::decoding(const char* src, char* dst, size_t size) {
	size_t len = strlen(src);
	if (len == -1) return -1;
	int x, y;
	x = (len & 0x3);
	if (x > 0 && x < 2) return -1;
	if ((len >> 2) * 3 + x - 1 > size)
		return -1;

	memset(dst, 0, size);
	
	int i = 0, j = 0;
	while (i < len && src[i] != padding_char) {
		x = find(src[i++]);
		if (x < 0) 
			return -1;

		y = find(src[i++]);
		if (y < 0)
			return -1;

		dst[j++] = ((x << 2) & 0xfc) | ((y >> 4) & 0x03);
		if (i >= len || src[i] == padding_char)
			return (y & 0x0f) ? -1 : j;
			
		x = find(src[i++]);
		if (x < 0)
			return -1;
			
		dst[j++] = ((x >> 2) & 0x0f) | ((y <<4) & 0xf0);
		if (i >= len || src[i] == padding_char)
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