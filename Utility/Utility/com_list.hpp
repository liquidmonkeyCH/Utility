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
template<class T,bool DuplicateAssert = false>
class list
{
public:
	class wrap;
	class iterator;
	class reverse_iterator;
	class node
	{
		friend class list;
		friend class wrap;
		friend class iterator;
		friend class reverse_iterator;
		node* m_prev{ nullptr };
		node* m_next{ nullptr };
		list* m_parent{ nullptr };

		inline void clear(void) { m_prev = m_next = nullptr; m_parent = nullptr; }
		inline operator bool(void) const { 
			if (DuplicateAssert) assert(m_prev == nullptr && m_next == nullptr);
			return (m_prev == nullptr && m_next == nullptr); 
		}

		T m_data;
	public:
		inline T& operator*(void) { return m_data; }
		inline T* super(void) { return &m_data; }
	};

	class wrap
	{
	public:
		friend class list;

		wrap(node* _node) :m_node(_node) {}
		inline T& operator*(void) { return m_node->m_data; }
		inline T* operator->(void) { return m_node ? &m_node->m_data : nullptr; }
		inline node* super(void) const { return m_node; }
	protected:
		node* m_node{ nullptr };
	};

	class iterator : public wrap
	{
	public:
		friend class list;
		friend class node;

		iterator(void) = default;
		iterator(node* _node) : wrap(_node) { if (_node) m_parent = _node->m_parent; }
		iterator(const wrap& w) : wrap(w) { if (this->m_node)m_parent = this->m_node->m_parent; }

		inline operator bool(void) const { return m_parent == nullptr ? false : true; }
		inline iterator& operator++(void) { next(); return *this; }
		inline iterator operator++(int) { node* _node = this->m_node; next(); return _node; }
		inline iterator& operator--(void) { prev(); return *this; }
		inline iterator operator--(int) { node* _node = this->m_node; prev(); return _node; }
		inline iterator& operator=(const iterator& it) { this->m_node = it.m_node; m_parent = it.m_parent; return *this; }
		inline iterator& operator=(node* _node) { this->m_node = _node; if(_node)m_parent = _node->m_parent; return *this; }
		inline iterator& operator=(const wrap& w) { this->m_node = w.m_node; if (w.m_node)m_parent = w.m_node->m_parent; return *this; }
		inline bool operator == (const iterator& it) { return it.m_parent == m_parent && it.m_node == this->m_node; }
		inline bool operator != (const iterator& it) { return !operator==(it); }
	protected:
		iterator(list* lst, node* _node) :wrap(_node), m_parent(lst) {}
		inline void next(void) { this->m_node = this->m_node ? this->m_node->m_next : m_parent->m_head; }
		inline void prev(void) { this->m_node = this->m_node ? this->m_node->m_prev : m_parent->m_tail; }
	protected:
		list* m_parent{ nullptr };
	};

	inline void push_front(node* p) {
		if (!p || !*p) return;
		if (m_head) m_head->m_prev = p, p->m_next = m_head,m_head = p;
		else m_head = m_tail = p;
		p->m_parent = this;
		++m_size;
	}

	inline void push_back(node* p) {
		if(!p || !*p) return;
		if (m_tail) m_tail->m_next = p, p->m_prev = m_tail,m_tail = p;
		else m_head = m_tail = p;
		p->m_parent = this;
		++m_size;
	}

	inline wrap pop_front(void) {
		if (!m_head) return nullptr;
		node* p = m_head;
		if (m_head->m_next) m_head = m_head->m_next, m_head->m_prev = nullptr;
		else m_head = m_tail = nullptr;
		p->node::clear();
		--m_size;
		return p;
	}

	inline wrap pop_back(void) {
		if (!m_tail) return nullptr;
		node* p = m_tail;
		if (m_tail->m_prev) m_tail = m_tail->m_prev, m_tail->m_next = nullptr;
		else m_head = m_tail = nullptr;
		p->node::clear();
		--m_size;
		return p;
	}

	inline iterator insert(const iterator& it, node* p) {
		if (!p || !*p) return { this, nullptr };
		assert(it.m_parent == this);
		node* _node = it.m_node;
		if (!_node || !*_node) {
			push_back(p);
			return { this,p };
		}

		p->m_parent = this;
		p->m_next = _node;
		p->m_prev = _node->m_prev;
		_node->m_prev = p;
		if (p->m_prev) p->m_prev->m_next = p;
		else m_head = p;
		++m_size;

		return { this,p };
	}

	inline iterator erase(const iterator& it) {
		assert(it.m_parent == this);
		node* _node = it.m_node;
		if (!_node || !*_node) return { this,nullptr };
		
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
		it.m_node->node::clear();
		--m_size;

		return { this,_node };
	}

	inline void clear(void) {
		while (m_head) { m_tail = m_head; m_head = m_head->m_next; m_tail->node::clear(); }
		m_tail = nullptr;
		m_size = 0;
	}

	inline iterator begin(void) { return { this, m_head }; }
	inline iterator end(void) { return { this,nullptr }; }
	inline reverse_iterator rbegin(void) { return { this, m_tail }; }
	inline reverse_iterator rend(void) { return { this,nullptr }; }

	inline wrap front(void) const { return m_head; }
	inline wrap back(void) const { return m_tail; }
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
