/**
* @file mem_stream.hpp
*
* cons
* @author Hourui (liquidmonkey)
*/
#ifndef __MEM_STREAM_HPP__
#define __MEM_STREAM_HPP__

#include "mem_buffer.hpp"
#include <iostream>

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace mem
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace impl
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class _stream_base
{
public:
	enum class state_t
	{
		eof = -1,			// 解包遇到文件尾
		good = 0,			// 正常
		over_flow,			// 压包溢出
	};
public:
	_stream_base(void) : m_state(state_t::good) {}
	~_stream_base(void) = default;

	inline state_t state(void) const { return m_state; }
	inline bool good(void) const { return m_state == state_t::good; }
protected:
	state_t	m_state;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class _buffer_stream : public _stream_base
{
public:
	_buffer_stream(buffer_iface* buffer) : m_buffer(buffer){}
	~_buffer_stream(void) = default;
protected:
	buffer_iface* m_buffer;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}
////////////////////////////////////////////////////////////////////////////////////////////////////
class memorystream : public impl::_stream_base
{
public:
	// 数据指针 数据最大长度 有效数据长度(写指针位置) 读指针位置
	memorystream(char* buf, std::size_t size, std::size_t len = 0, std::size_t pos = 0)  
		: m_buffer(buf)
		, m_max(size)
		, m_len(len)
		, m_pos(pos) {}
	~memorystream(void) = default;

	inline void reset(char* buf, std::size_t size, std::size_t len = 0, std::size_t pos = 0) {
		m_buffer = buf;
		m_max = size;
		m_len = len;
		m_pos = pos;
		m_state = state_t::good;
	}

	inline std::size_t size(void) const { return m_len; }
	inline std::size_t max(void) const { return m_max; }
	inline const char* data(void) const { return m_buffer; }
public:
	bool set(const char* buf, std::size_t size, std::size_t pos = 0) {
		std::size_t len = pos + size;
		if (len > m_max) {
			m_state = state_t::over_flow;
			return false;
		}
		if (len > m_len) m_len = len;
		memcpy(m_buffer + pos, buf, size);
		return true;
	}

	template<class T>
	inline bool set(T& data, std::size_t pos = 0) {
		return set((const char*)&data, sizeof(data), pos);
	}

	inline bool append(const char* buf, std::size_t size) {
		return set(buf, size, m_len);
	}

	inline bool append(memorystream& stream) {
		return set(stream.data(), stream.size(), m_len);
	}

	template<class T>
	memorystream& operator<<(const T& data) {
		append((const char*)&data, sizeof(data));
		return *this;
	}

	bool get(char* buffer, std::size_t size, std::size_t pos = 0) {
		if (pos + size > m_len) return false;
		memcpy(buffer, m_buffer + pos, size);
		return true;
	}

	template<class T>
	memorystream& operator>>(T& data) {
		std::size_t pos = m_pos + sizeof(data);
		if (pos > m_len) {
			m_state = state_t::eof;
			return *this;
		}	
		memcpy(&data, m_buffer + m_pos, sizeof(data));
		m_pos = pos;
		return *this;
	}

	bool skip(std::size_t n) {
		std::size_t pos = m_pos + n;
		if (pos > m_len) {
			m_state = state_t::eof;
			return false;
		}
		m_pos = pos;
		return true;
	}
private:
	char* m_buffer;
	std::size_t m_max;
	std::size_t m_len;
	std::size_t m_pos;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class istream : virtual public impl::_buffer_stream
{
public:
	template<class T>
	istream& operator << (T& data);
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class ostream : virtual public impl::_buffer_stream
{
public:
	template<class T>
	ostream& operator >> (T& data);
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class iostream : public istream, public ostream {};
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
istream& istream::operator<<(T& kData)
{
	std::size_t size = sizeof(T);

	if (m_buffer->writable_size() < size)
	{
		m_state = state_t::over_flow;
		return *this;
	}

	std::size_t len,pos = 0;
	const char* p;

	do {
		len = size - pos;
		p = m_buffer->write(len);
		memcpy(p, &kData + pos, len);
		m_buffer->commit_write(len);
		pos += len;
	} while (pos < size);
	
	return *this;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
ostream& ostream::operator>>(T& kData)
{
	std::size_t size = sizeof(T);

	if (m_buffer->readable_size() < size)
	{
		m_state = state_t::eof;
		return *this;
	}

	std::size_t len,pos = 0;
	const char* p;

	do {
		len = size - pos;
		p = m_buffer->read(len);
		memcpy(&kData + pos, p, len);
		m_buffer->commit_read(len);
		pos += len;
	} while (pos < size);

	return *this;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace mem
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__MEM_BUFFER_HPP__
