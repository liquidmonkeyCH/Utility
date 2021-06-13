/**
* @file mem_recorder.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __MEM_RECORDER_HPP__
#define __MEM_RECORDER_HPP__

#include "mem_data_factory.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace mem
{
////////////////////////////////////////////////////////////////////////////////////////////////////
template<std::size_t block_size>
class recorder
{
private:
	static constexpr int max_len = block_size * 10;
	struct node_t
	{
		inline void clear(void) { m_reader = m_buffer;  m_writer = m_buffer; m_next = nullptr; }
		char m_buffer[max_len + block_size + 2] = {0};
		const char* m_tail = m_buffer + max_len;
		char* m_reader = m_buffer;
		char* m_writer = m_buffer;
		node_t* m_next = nullptr;
	};
public:
	recorder(void) : m_reader_node(m_pool.malloc()), m_writer_node(m_reader_node), m_last_read(0) {}
	~recorder(void) = default;

	const char* read(std::size_t& size){
		do{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_last_read = m_reader_node->m_writer - m_reader_node->m_reader;
		}while (false);
		
		size = m_last_read;
		return m_reader_node->m_reader;
	}
	void commit_read(void) {
		m_reader_node->m_reader += m_last_read;
		if (m_reader_node->m_reader > m_reader_node->m_tail){
			node_t* node = m_reader_node;
			m_reader_node = m_reader_node->m_next;
			node->clear();
			std::lock_guard<std::mutex> lock(m_mutex);
			m_pool.free(node);
		}
	}
	inline char* write(size_t* size = nullptr) const { 
		if (size) *size = m_writer_node->m_tail - m_writer_node->m_writer + block_size;
		return m_writer_node->m_writer;
	}
	bool commit_write(std::size_t size) {
		std::lock_guard<std::mutex> lock(m_mutex);
		m_writer_node->m_writer += size;
		if (m_writer_node->m_writer > m_writer_node->m_tail){
			m_writer_node->m_next = m_pool.malloc();
			m_writer_node = m_writer_node->m_next;
		}

		return 0 == m_last_read;
	}
private:
	std::mutex m_mutex;
	mem::data_pool<node_t, 5> m_pool;
	node_t* m_reader_node;
	node_t* m_writer_node;
	std::ptrdiff_t m_last_read;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace mem
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__MEM_RECORDER_HPP__