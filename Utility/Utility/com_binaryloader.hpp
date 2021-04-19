/**
* @file com_binaryloader.hpp
*
* @author Hourui (liquidmonkey)
**/

#ifndef __COM_BINARY_LOADER_HPP__
#define __COM_BINARY_LOADER_HPP__

#include <fstream>
#include "mem_stream.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////
namespace com
{
////////////////////////////////////////////////////////////////////////////////
class binaryloader : public mem::memorystream
{
public:
	binaryloader(void) : memorystream(nullptr, 0), m_buffer(nullptr) {}
	~binaryloader(void) { if (m_buffer) delete[] m_buffer; }
public:
	bool open(const char* file) {
		m_file.clear();
		m_file.open(file, std::ios::in | std::ios::binary);
		if (!m_file.is_open()) return false;
		std::streambuf* buf = m_file.rdbuf();
		std::size_t len = buf->pubseekoff(0, std::ios::end, std::ios::in);
		std::size_t max = size();
		buf->pubseekpos(0, std::ios::in);
		if (len > max) {
			if (m_buffer) delete[] m_buffer;
			m_buffer = new char[len];
			max = len;
		}
		buf->sgetn(m_buffer, len);
		reset(m_buffer, max, len);
		return true;
	}
private:
	char* m_buffer;
	std::ifstream m_file;
};
////////////////////////////////////////////////////////////////////////////////
}// namespace com 
////////////////////////////////////////////////////////////////////////////////
}// namespace Utility
#endif //__COM_BINARY_LOADER_HPP__
