/**
* @file mem_message.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __MEM_MESSAGE_HPP__
#define __MEM_MESSAGE_HPP__

#include "base_defines.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace mem
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class message
{
public:
	message(void) :m_limit(0), m_position(0) {}
	virtual ~message(void) = default;
 
	// Read a number of bytes.
	virtual const char* next(net_size_t &size) = 0;
	// Skip a number of bytes.
	virtual bool skip(net_size_t size) = 0;
	// Backs up a number of bytes.
	virtual bool back_up(net_size_t size) = 0;

	// Reset total number of bytes read since this object was created to zero.
	virtual void reset(void) { m_position = 0; }
	// Returns the total number of bytes read since this object was created.
	net_size_t get_read_bytes(void) const { return m_position; }
	net_size_t get_read_limit(void) const { return m_limit; }

	template<class T>
	bool get(T& data)
	{
		net_size_t len = sizeof(T);
		const char* p;
		char* head = (char*)&data;
		const char* tail = head + len;
		do
		{
			p = this->next(len);
			if (!p) return false;
			memcpy(head, p, len);
			head += len;
			if (head >= tail) break;
			len = net_size_t(tail - head);
		} while (true);
		return true;
	}
protected:
	void set_read_limit(net_size_t limit) { m_limit = limit; }
protected:
	net_size_t m_limit;
	net_size_t m_position;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace mem
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__MEM_BUFFER_HPP__