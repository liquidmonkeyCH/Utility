/**
* @file com_list.hpp
*
* @author Hourui (liquidmonkey)
**/

#ifndef __COM_LIST_HPP__
#define __COM_LIST_HPP__

#include <assert.h>

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////
namespace com
{
////////////////////////////////////////////////////////////////////////////////
template<class T>
class list
{
public:
	class node
	{
		friend class list;
		node* m_prev{ nullptr };
		node* m_next{ nullptr };

		inline void clear(void) { m_prev = m_next = nullptr; }
		inline operator bool(void) const { return (m_prev == nullptr && m_next == nullptr); }
	};

	class iterator
	{
	public:
		friend class list;
		iterator(list* lst, node* node) :m_parent(lst), m_node(node) {}

		inline iterator& operator=(const iterator& it) { m_node = it.m_node; m_parent = it.m_parent; }
		inline T* operator*(void) { return (T*)m_node; }
		inline iterator& operator++(void) { next(); return *this; }
		inline iterator operator++(int) { iterator r(*this); next(); return std::move(r); }
		inline iterator& operator--(void) { prev(); return *this; }
		inline iterator operator--(int) { iterator r(*this); prev(); return std::move(r); }
	private:
		inline void next(void) { if (m_node) m_node = m_node->m_next; }
		inline void prev(void) { if (!m_node) m_node = m_parent->m_tail; else if (m_node->m_prev) m_node = m_node->m_prev; }
	private:
		list* m_parent{ nullptr };
		node* m_node{ nullptr };
	};

	inline void push_front(node* p) {
		assert(*p);
		if (m_head) m_head->m_prev = p, p->m_next = m_head;
		else m_head = m_tail = p;
		++m_size;
	}

	inline void push_back(node* p) {
		assert(*p);
		if (m_tail) m_tail->m_next = p, p->m_prev = m_tail;
		else m_head = m_tail = p;
		++m_size;
	}

	inline T* pop_front(void) {
		if (!m_head) return nullptr;
		node* p = m_head;
		if (m_head->m_next) m_head = m_head->m_next, m_head->m_prev = nullptr;
		else m_head = m_tail = nullptr;
		p->m_next = nullptr;
		--m_size;
		return p;
	}

	inline T* pop_back(void) {
		if (!m_tail) return nullptr;
		node* p = m_tail;
		if (m_tail->m_prev) m_tail = m_tail->m_prev, m_tail->m_next = nullptr;
		else m_head = m_tail = nullptr;
		p->m_prev = nullptr;
		--m_size;
		return p;
	}

	inline iterator insert(const iterator& it, node* p) {
		assert(*p);
		assert(it.m_parent = this);
		node* _node = it.m_node;
		if (!_node) {
			push_back(p);
			return { this,p };
		}

		p->m_next = _node;
		p->m_prev = _node->m_prev;
		_node->m_prev = p;
		if (p->m_prev) p->m_prev->m_next = p;
		++size;

		return { this,p };
	}

	inline iterator erase(const iterator& it) {
		assert(it.m_parent = this);
		node* _node = it.m_node;
		if (!_node) return { this,nullptr };
		
		if (_node == m_tail) {
			pop_back();
			return { this,nullptr };
		}

		if (_node == m_head) {
			pop_front();
			return { this,m_head };
		}

		_node->m_prev->m_next = _node->m_next;
		_node->m_next->m_prev = _node->m_prev;
		_node = _node->m_next;
		it.m_node->clear();

		return { this,_node };
	}

	inline iterator begin(void) { return { this, m_head }; }
	inline iterator end(void) { return { this,nullptr }; }

	inline T* front(void) { return (T*)m_head; }
	inline T* back(void) { return (T*)m_tail; }
	inline size_t size(void) const { return m_size; }
	inline bool empty(void) const { return m_size == 0; }
private:
	node* m_head{ nullptr };
	node* m_tail{ nullptr };
	size_t m_size{ 0 };
};
////////////////////////////////////////////////////////////////////////////////
}// namespace com 
////////////////////////////////////////////////////////////////////////////////
}// namespace Utility
#endif //__COM_LIST_HPP__
