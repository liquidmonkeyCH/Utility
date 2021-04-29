/**
* @file mem_data_factory.hpp
*
* @author Hourui (liquidmonkey)
*/

#ifndef __MEM_DATA_FACTORY_HPP__
#define __MEM_DATA_FACTORY_HPP__

#include <assert.h>
#include <limits>
#include <climits>
#include <vector>
#include <mutex>
#include <queue>
#include <atomic>
#include <string.h>

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace mem 
{ 
enum factory_cache_type { NO_CACHE, DYNAMIC }; 
template<class, std::uint64_t, std::uint64_t, size_t> class data_pool;
} 
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace _impl
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace factory
{
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, std::uint64_t N>
class data_factory
{
public:
	using size_type = typename std::conditional < N == 0, size_t,
		typename std::conditional < N < UCHAR_MAX, unsigned char,
		typename std::conditional<N < USHRT_MAX, unsigned short,
		typename std::conditional<N < ULONG_MAX, unsigned long,
		unsigned long long>::type>::type>::type>::type;

	static constexpr size_type _zero = size_type(-1);
	static constexpr size_type _used = _zero - 1;
	static_assert(N < _used, "size out of range!");

	template<class, std::uint64_t, std::uint64_t, size_t> friend class mem::data_pool;
public:
	class iterator
	{
	public:
		iterator(data_factory* data, size_type pos) :m_parent(data), m_pos(pos) {}
	public:
		T* operator->(void) { return m_parent->m_data + m_pos; }
		T* operator*(void) { return m_parent->m_data + m_pos; }
		iterator operator++(int);
		iterator& operator++(void);
		iterator operator--(int);
		iterator& operator--(void);
		bool operator==(const iterator& rhs) { return m_parent == rhs.m_parent && m_pos == rhs.m_pos; }
		bool operator!=(const iterator& rhs) { return !operator==(rhs); }
	private:
		data_factory*	m_parent;
		size_type		m_pos;
	};
	friend class iterator;
public:
	T* malloc(void);
	bool free(T* p);

	iterator used_begin(void);
	iterator used_end(void);

	size_type size(void) { return m_size; }
	size_type left(void) { return m_left; }
	size_type used(void) { return m_size - m_left; }
protected:
	size_type npos(T* p);
	bool _free(T* p,bool force = true);
	size_type head_zero(void);
	T* head_data(void) { return m_data; }
protected:
	T*			m_data;
	size_type*	m_offset;
	size_type	m_next, m_left, m_size;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class Factory, size_t Cache>
class allocator
{
public:
	using size_type = typename Factory::size_type;

	allocator(void) { m_head = m_cache + Cache; m_tail = m_head; }

	Factory* malloc(size_type size);
	void free(Factory* p);
private:
	std::mutex	m_mutex;
	Factory* m_cache[Cache];
	Factory** m_head;
	Factory** m_tail;
};

template<class Factory>
class allocator<Factory, mem::factory_cache_type::NO_CACHE>
{
public:
	using size_type = typename Factory::size_type;

	Factory* malloc(size_type size);
	void free(Factory* p);
};

template<class Factory>
class allocator<Factory, mem::factory_cache_type::DYNAMIC>
{
public:
	using size_type = typename Factory::size_type;
	allocator(void):m_size(10){}

	void set_cache(size_t size) { m_size = size; }
	Factory* malloc(size_type size);
	void free(Factory* p);
private:
	std::mutex				m_mutex;
	std::queue<Factory*>	m_cache;
	std::atomic<size_t>		m_size;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class Factory, size_t Cache>
class allocator_wrap
{
protected:
	using allocator_t = allocator<Factory, Cache>;
	static allocator_t* get_allocator(void) { static allocator_t _allocator; return &_allocator; }
};

template<class Factory>
class allocator_wrap<Factory,mem::factory_cache_type::DYNAMIC>
{
protected:
	using allocator_t = allocator<Factory, mem::factory_cache_type::DYNAMIC>;
	static allocator_t* get_allocator(void) { static allocator_t _allocator; return &_allocator; }
public:
	static void set_cache(size_t size) { get_allocator()->set_cache(size); }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class Factory>
class thread_safe
{
	using value_t = typename Factory::value_t;
public:
	thread_safe(void) = default;
	~thread_safe(void) { clear(); }

	inline void init(size_t size = 0, size_t grow = 0){
		static std::once_flag oc;
		std::call_once(oc, [this]() { this->m_factory.init(size, grow); });
	}

	inline auto malloc(void)->value_t* {
		std::lock_guard<std::mutex> lock(m_mutex);
		return this->m_factory.malloc();
	}

	inline bool free(value_t* p){
		std::lock_guard<std::mutex> lock(m_mutex);
		return this->m_factory.free(p);
	}

	inline void clear(void){
		std::lock_guard<std::mutex> lock(m_mutex);
		this->m_factory.clear();
	}
private:
	std::mutex m_mutex;
	Factory m_factory;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}// namespace factory
////////////////////////////////////////////////////////////////////////////////////////////////////
}// namespace _impl
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace mem
{
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, std::uint64_t N = 0>
class data_factory : public _impl::factory::data_factory<T,N>
{
public:
	using value_t = T;
	using base = _impl::factory::data_factory<T,N>;
	using size_type = typename base::size_type;
	using iterator = typename base::iterator;

	T				_data[N];
	size_type		_offset[N];
public:
	data_factory(void);
	~data_factory(void);

	data_factory(const data_factory&) = delete;
	data_factory& operator=(const data_factory&) = delete;

	data_factory(const data_factory&&) = delete;
	data_factory& operator=(const data_factory&&) = delete;

	void init(size_t size = 0) { (void)size; clear(); }
	void clear(void);
};

template<class T>
class data_factory<T, 0> : public _impl::factory::data_factory<T, 0>
{
public:
	using base = _impl::factory::data_factory<T,0>;
	using size_type = typename base::size_type;
	using iterator = typename base::iterator;
public:
	data_factory(void);
	~data_factory(void);

	data_factory(const data_factory&) = delete;
	data_factory<T, 0>& operator=(const data_factory&) = delete;

	data_factory(const data_factory&&) = delete;
	data_factory<T, 0>& operator=(const data_factory&&) = delete;

	void init(size_t size);
	void clear(void);
};
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, std::uint64_t N = 0, std::uint64_t G = 0, size_t Cache = factory_cache_type::NO_CACHE>
class data_pool : public
	_impl::factory::allocator_wrap< data_factory<T, G>, Cache>
{
public:
	using value_t = T;
	using base_t = data_factory<T, G>;
	using main_t = data_factory<T, N>;
	using allocator_t = _impl::factory::allocator<base_t, Cache>;
	using chunks_t = std::vector<base_t*>;
	using size_type = typename base_t::size_type;
	using allocator_wrap_t = _impl::factory::allocator_wrap<base_t, Cache>;
private:
	static constexpr size_t main_trunk = size_t(-1);
	chunks_t	m_chunks;
	size_t		m_alloc_chunk;
	size_t		m_dealloc_chunk;
	size_type	m_grow;
	main_t		m_main_trunk;
private:
	auto npos(T* p, size_t& n)->size_type;
public:
	data_pool(void);
	~data_pool(void);

	data_pool(const data_pool&) = delete;
	data_pool& operator=(const data_pool&) = delete;

	data_pool(const data_pool&&) = delete;
	data_pool& operator=(const data_pool&&) = delete;

	void init(size_t size = 0, size_t grow = 0);
	void clear(void);

	T* malloc(void);
	bool free(T* p);
};
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, std::uint64_t N = 0, std::uint64_t G = 0, size_t Cache = factory_cache_type::NO_CACHE>
using data_pool_s = _impl::factory::thread_safe<data_pool<T, N, G, Cache>>;
template<class T, std::uint64_t N = 0>
using data_factory_s = _impl::factory::thread_safe<data_factory<T, N>>;
////////////////////////////////////////////////////////////////////////////////////////////////////
} // mem
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "mem_data_factory.inl"
////////////////////////////////////////////////////////////////////////////////////////////////////	
}//namespace Utility
#endif //__MEM_DATA_FACTORY_HPP__