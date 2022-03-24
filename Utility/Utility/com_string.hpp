/**
* @file com_string.hpp
*
* @author Hourui (liquidmonkey)
*/

#ifndef __COM_STRING_HPP__
#define __COM_STRING_HPP__

#include <string>
#include <assert.h>
#include "com_hex_caster.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace com
{
////////////////////////////////////////////////////////////////////////////////////////////////////
template<size_t N>
class ustring
{
	static constexpr size_t max_len = N + 2;
	inline void init(void) { if (m_len > N) m_len = N; }
public:
	ustring(void) = default;
	ustring(const char* p) :m_len(strlen(p)) { init(); memcpy(m_data, p, m_len); }
	ustring(const std::string& str) :ustring(str.c_str()) {}
	template<size_t M>
	ustring(const ustring<M>& r) : m_len(r.size()) { init(); memcpy(m_data, r.c_str(), m_len); }
	template<size_t M>
	ustring& operator=(const ustring<M>& r) { m_len = r.size(); init(); memcpy(m_data, r.c_str(), m_len); }

	inline size_t size(void) const { return m_len; }
	inline bool empty(void) const { return m_len == 0; }
	inline ustring& clear(void) { m_len = 0; memset(m_data, 0, max_len); return *this; }
	inline const char* c_str(void) const { return m_data; }
	inline const char& at(size_t off) { assert(off < N); return m_data[off]; }
	inline const char& operator[](size_t off) { return at(off); }

	ustring& format(const char* fmt, ...) {
		va_list arglist;
		va_start(arglist, fmt);
		vsnprintf(m_data, N, fmt, arglist);
		va_end(arglist);
		size_t len = strlen(m_data);
		if(m_len > len) memset(m_data + len, 0, m_len - len);
		m_len = len;
		return *this;
	}

	ustring& append(const char* fmt, ...) {
		if (m_len >= N) return *this;
		char* p = m_data + m_len;
		va_list arglist;
		va_start(arglist, fmt);
		vsnprintf(p, N - m_len, fmt, arglist);
		va_end(arglist);
		m_len += strlen(p);
		return *this;
	}

	ustring& append(size_t n, const char& c) {
		if (m_len >= N) return *this;
		size_t left = N - m_len;
		if (n > left) n = left;
		memset(m_data + m_len, c, n);
		m_len += n;
		return *this;
	}

	ustring& append(size_t len, const char* str) {
		if (m_len >= N) return *this;
		size_t left = N - m_len;
		if (len > left) len = left;
		memcpy(m_data + m_len, str, len);
		m_len += len;
		return *this;
	}

	template<class T>
	inline ustring& operator << (const T& data) { append(com::bin2hex(data).c_str()); return *this; }
	template<> inline ustring& operator << (const uint64_t& n) { return append("%llu", n); }
	template<> inline ustring& operator << (const int64_t& n) { return append("%lld", n); }
	template<> inline ustring& operator << (const uint32_t& n) { return append("%u", n); }
	template<> inline ustring& operator << (const int32_t& n) { return append("%d", n); }
	template<> inline ustring& operator << (const uint16_t& n) { return append("%u", n); }
	template<> inline ustring& operator << (const int16_t& n) { return append("%d", n); }
	template<> inline ustring& operator << (const uint8_t& n) { return append("%u", n); }
	template<> inline ustring& operator << (const int8_t& n) { return append("%d", n); }
	template<> inline ustring& operator << (const float& n) { return append("%f", n); }
	template<> inline ustring& operator << (const double& n) { return append("%llf", n); }
	inline ustring& operator << (const char* str) { return append(strlen(str), str); }
	
	inline ustring& copy(const char* src, size_t count) {
		if (count > N) count = N;
		memcpy(m_data, src, count);
		if(m_len > count) memset(m_data + m_len, 0, m_len - count);
		m_len = count;
		return *this;
	}

	inline ustring& pop(size_t n) {
		if (m_len <= n) return clear();
		m_len -= n;
		memset(m_data + m_len, 0, n);
		return *this;
	}

	inline ustring& sub(size_t len, size_t off = 0) {
		if (m_len <= off || len == 0) return clear();
		char* p = m_data + off;
		off = m_len - off;
		if (len > off) len = off;
		memcpy(m_data, p, len);
		memset(m_data + len, 0, m_len - len);
		m_len = len;
		return *this;
	}

	inline ustring substr(size_t count, size_t off = 0) { return ustring(*this).sub(count,off); }
private:
	char m_data[max_len]{ 0 };
	size_t m_len{ 0 };
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}// namespace com
////////////////////////////////////////////////////////////////////////////////////////////////////
}// namespace Utility

#endif //!__COM_STRING_HPP__