/**
* @file com_character.hpp
*
* @author Hourui (liquidmonkey)
**/

#ifndef __COM_CHARACTER_HPP__
#define __COM_CHARACTER_HPP__

#include <iostream>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <cstddef>

#ifdef _WIN32
#include <windows.h>
#else
#include <iconv.h>
#endif

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////
namespace com
{
////////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
int GbkToUtf8(const char* str_str, char* dst_str, size_t size){
	int len = MultiByteToWideChar(CP_ACP, 0, str_str, -1, NULL, 0);
	if (len <= 0)
		return -1;

	std::wstring wstr(size_t(len),L'\0');
	wchar_t* pstr = const_cast<wchar_t*>(wstr.c_str());
	MultiByteToWideChar(CP_ACP, 0, str_str, -1, pstr, len);
	len = WideCharToMultiByte(CP_UTF8, 0, pstr, -1, NULL, 0, NULL, NULL);
	if (len > size)
		return -1;

	memset(dst_str, 0, size_t(len));
	WideCharToMultiByte(CP_UTF8, 0, pstr, -1, dst_str, len, NULL, NULL);
	
	return 0;
}

int Utf8ToGbk(const char* src_str, char* dst_str, size_t size) {
	int len = MultiByteToWideChar(CP_UTF8, 0, src_str, -1, NULL, 0);
	if (len <= 0)
		return -1;

	std::wstring wstr(size_t(len), L'\0');
	wchar_t* pstr = const_cast<wchar_t*>(wstr.c_str());
	MultiByteToWideChar(CP_UTF8, 0, src_str, -1, pstr, len);
	len = WideCharToMultiByte(CP_ACP, 0, pstr, -1, NULL, 0, NULL, NULL);

	if (len > size)
		return -1;

	memset(dst_str, 0, size_t(len));
	WideCharToMultiByte(CP_ACP, 0, pstr, -1, dst_str, len, NULL, NULL);

	return 0;
}
#else
int GbkToUtf8(const char* src_str, char* dst_str, size_t size){
	iconv_t cd;
	size_t src_len = strlen(src_str);
	const char** pin = &src_str;
	char** pout = &dst_str;

	cd = iconv_open("utf8", "gbk");
	if (cd == 0)
		return -1;
	memset(dst_str, 0, size);
	if (iconv(cd, pin, &src_len, pout, &size) == -1)
		return -1;
	iconv_close(cd);
	*pout = '\0';

	return 0;
}

int Utf8ToGbk(const char* src_str, char* dst_str, size_t size){
	iconv_t cd;
	size_t src_len = strlen(src_str);
	const char** pin = &src_str;
	char** pout = &dst_str;

	cd = iconv_open("gbk", "utf8");
	if (cd == 0)
		return -1;
	memset(dst_str, 0, size);
	if (iconv(cd, pin, &src_len, pout, &size) == -1)
		return -1;
	iconv_close(cd);
	*pout = '\0';

	return 0;
}
#endif
}
}
#endif