// MIT License
//
// Copyright (c) 2016-2017 Simon Ninon <simon.ninon@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

/**
* @file logger.cpp
*
* @author Hourui (liquidmonkey)
*/

#include "Utility/logger.hpp"
#include <iostream>

////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#include <windows.h>
#include <functional>
#define DECLARE_COLOR(color,flag)							\
	inline std::ostream& color(std::ostream &s)				\
	{														\
		HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);	\
		SetConsoleTextAttribute(hStdout,					\
		flag | FOREGROUND_INTENSITY);						\
		return s;											\
	}

DECLARE_COLOR(red, FOREGROUND_RED)
DECLARE_COLOR(green, FOREGROUND_GREEN)
DECLARE_COLOR(blue, FOREGROUND_BLUE)
DECLARE_COLOR(cyan, FOREGROUND_BLUE | FOREGROUND_GREEN)
DECLARE_COLOR(yellow, FOREGROUND_GREEN | FOREGROUND_RED)
DECLARE_COLOR(purple, FOREGROUND_BLUE | FOREGROUND_RED)
DECLARE_COLOR(normal, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#undef DECLARE_COLOR
#else
static const char red[] = { 0x1b, '[', '1', ';', '3', '1', 'm', 0 };
static const char green[] = { 0x1b, '[', '1', ';', '3', '2', 'm', 0 };
static const char blue[] = { 0x1b, '[', '1', ';', '3', '4', 'm', 0 };
static const char yellow[] = { 0x1b, '[', '1', ';', '3', '3', 'm', 0 };
static const char purple[] = { 0x1b, '[', '1', ';', '3', '5', 'm', 0 };
static const char cyan[] = { 0x1b, '[', '1', ';', '3', '6', 'm', 0 };
static const char normal[] = { 0x1b, '[', '0', ';', '3', '9', 'm', 0 };
#endif

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
void logger::output(log_type t, const char* head, const char* str){
	int n = static_cast<int>(t);
	std::lock_guard<std::mutex> lock(m_mutex);
	switch (t) {
	case log_type::error:
		std::cout << "[" << red << log_title[n] << normal << "]";
		break;
	case log_type::warn:
		std::cout << "[" << yellow << log_title[n] << normal << "]";
		break;
	case log_type::info:
		std::cout << "[" << cyan << log_title[n] << normal << "]";
		break;
	case log_type::echo:
		std::cout << "[" << green << log_title[n] << normal << "]";
		break;
	case log_type::dump:
		std::cout << "[" << blue << log_title[n] << normal << "]";
		break;
	case log_type::debug:
		std::cout << "[" << purple << log_title[n] << normal << "]";
		break;
	default:
		std::cout << "[" << purple << "????" << normal << "]";
		break;
	}

	std::cout << head << " " << str << std::endl;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
logger_iface* Clog::m_logger = nullptr;
std::uint8_t Clog::m_level = Clog::level::debug;
std::uint8_t Clog::m_warn = Clog::level::debug;
std::hash<std::thread::id> Clog::m_hash = std::hash<std::thread::id>();
////////////////////////////////////////////////////////////////////////////////////////////////////
}// namespace Utility