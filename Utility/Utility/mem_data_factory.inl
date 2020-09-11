/**
* @file mem_data_factory.inl
*
* @author Hourui (liquidmonkey)
*/

namespace _impl
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace factory
{
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, std::uint64_t N>
inline typename data_factory<T, N>::size_type
data_factory<T, N>::npos(T* p)
{
	if (!m_data || p < m_data || p >= m_data + m_size) return _zero;

	return size_type(p - m_data);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, std::uint64_t N>
inline T*
data_factory<T, N>::malloc(void)
{
	if (!m_left) return nullptr;

	--m_left;
	size_type next = m_next;
	m_next = m_offset[m_next] ? (m_offset[m_next] == _zero ? 0 : m_offset[m_next]) : (m_next + 1);

	m_offset[next] = _used;
	return m_data + next;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, std::uint64_t N>
inline bool
data_factory<T, N>::free(T* p)
{
	return _free(p, false);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, std::uint64_t N>
inline bool
data_factory<T, N>::_free(T* p,bool force)
{
	size_type _pos = npos(p);
	if(!force) assert(_pos != _zero);
	if (_pos == _zero) return false;

	if (!force) assert(m_offset[_pos] == _used);
	if (m_offset[_pos] != _used) return true;
	++m_left;

	m_offset[_pos] = m_next ? m_next : _zero;
	m_next = _pos;

	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, std::uint64_t N>
inline typename data_factory<T, N>::iterator
data_factory<T, N>::used_begin(void)
{
	if (m_offset[0] == _used) return{ this, 0 };
	if (m_offset[0] == 0) return{ this, m_size };

	iterator it(this, 0);
	return ++it;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, std::uint64_t N>
inline typename data_factory<T, N>::iterator
data_factory<T, N>::used_end(void)
{
	return{ this, m_size };
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, std::uint64_t N>
inline  typename data_factory<T, N>::size_type
data_factory<T, N>::head_zero(void)
{
	size_type head = 0;
	size_type tail = m_size;
	size_type n;
	do {
		n = (head + tail) / 2;
		if (m_offset[n] == 0)
		{
			if (n == head)	return n;
			tail = n;
		}
		else
		{
			if (n == head) { return n + 1; }
			head = n;
		}
	} while (true);

	return n;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, std::uint64_t N>
inline typename data_factory<T, N>::iterator&
data_factory<T, N>::iterator::operator++(void)
{
	while (m_pos < m_parent->m_size && m_parent->m_offset[++m_pos] != m_parent->_used) {
		if (m_parent->m_offset[m_pos] == 0) {
			m_pos = m_parent->m_size;
			break;
		}
	}

	return *this;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, std::uint64_t N>
inline typename data_factory<T, N>::iterator
data_factory<T, N>::iterator::operator++(int)
{
	iterator it = *this;
	operator++();
	return it;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, std::uint64_t N>
inline typename data_factory<T, N>::iterator&
data_factory<T, N>::iterator::operator--(void)
{
	if (m_parent->m_offset[0] == 0) return *this;
	if (m_pos == m_parent->m_size) m_pos = m_parent->head_zero();

	while (m_pos > 0 && m_parent->m_offset[--m_pos] != m_parent->_used) {}
	return *this;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, std::uint64_t N>
inline typename data_factory<T, N>::iterator
data_factory<T, N>::iterator::operator--(int)
{
	iterator it = *this;
	operator--();
	return it;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class Factory, size_t Cache>
inline Factory*
allocator<Factory, Cache>::malloc(typename allocator<Factory, Cache>::size_type size)
{
	Factory* p = nullptr;
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_head != m_tail) p = *m_head++;
	}
	if (!p) p = new Factory;
	p->init(size);
	return p;
}

template<class Factory, size_t Cache>
inline void
allocator<Factory, Cache>::free(Factory* p)
{
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_head > m_cache)
		{
			*--m_head = p;
			return;
		}
	}
	delete p;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class Factory>
inline Factory*
allocator<Factory, mem::factory_cache_type::NO_CACHE>::malloc(typename allocator<Factory, mem::factory_cache_type::NO_CACHE>::size_type size)
{
	Factory* p = new Factory;
	p->init(size);
	return p;
}

template<class Factory>
inline void
allocator<Factory, mem::factory_cache_type::NO_CACHE>::free(Factory* p)
{
	delete p;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class Factory>
inline Factory*
allocator<Factory, mem::factory_cache_type::DYNAMIC>::malloc(typename allocator<Factory, mem::factory_cache_type::DYNAMIC>::size_type size)
{
	Factory* p = nullptr;
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (!m_cache.empty())
		{
			p = m_cache.front();
			m_cache.pop();
		}
	}
	if (!p) p = new Factory;
	p->init(size);
	return p;
}

template<class Factory>
inline void
allocator<Factory, mem::factory_cache_type::DYNAMIC>::free(Factory* p)
{
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_cache.size() < m_size)
		{
			m_cache.push(p);
			return;
		}
	}
	delete p;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
}// namspace factory
////////////////////////////////////////////////////////////////////////////////////////////////////
}// namespace _impl
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace mem
{
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, std::uint64_t N>
inline
data_factory<T, N>::data_factory(void)
{
	this->m_offset = _offset;
	this->m_data = _data;
	this->m_size = N;

	clear();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, std::uint64_t N>
inline
data_factory<T, N>::~data_factory(void)
{
	clear();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, std::uint64_t N>
inline
void data_factory<T, N>::clear(void)
{
	this->m_next = 0;
	this->m_left = N;
	memset(_offset, 0, sizeof(size_type) * N);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
inline
data_factory<T, 0>::data_factory(void)
{
	this->m_data = nullptr;
	this->m_offset = nullptr;
	this->m_next = 0;
	this->m_left = 0;
	this->m_size = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
inline
data_factory<T, 0>::~data_factory(void)
{
	delete[] this->m_data;
	delete[] this->m_offset;

	this->m_data = nullptr;
	this->m_offset = nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
inline void 
data_factory<T, 0>::clear(void)
{
	this->m_next = 0;
	this->m_left = this->m_size;

	memset(this->m_offset, 0, sizeof(size_type) * size_t(this->m_size));
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
inline void
data_factory<T, 0>::init(typename data_factory<T, 0>::size_type size)
{
	assert(size != 0);
	assert(size < this->_used);
	if (this->m_data != nullptr)
	{
		if (size == this->m_size)
		{
			clear();
			return;
		}

		delete[] this->m_data;
		delete[] this->m_offset;
	}

	this->m_data = new T[size];
	this->m_offset = new size_type[size];
	this->m_size = size;
	clear();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, std::uint64_t N, std::uint64_t G, size_t Cache>
data_pool<T, N, G, Cache>::data_pool(void)
	: m_alloc_chunk(main_trunk)
	, m_dealloc_chunk(main_trunk)
	, m_grow(G)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, std::uint64_t N, std::uint64_t G, size_t Cache>
data_pool<T, N, G, Cache>::~data_pool(void)
{
	clear();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, std::uint64_t N, std::uint64_t G, size_t Cache>
inline void
data_pool<T, N, G, Cache>::init(typename data_pool<T, N, G, Cache>::size_type size, typename data_pool<T, N, G, Cache>::size_type grow)
{
	m_main_trunk.init(size);
	if (m_grow > 0) return;
	if (grow == 0) grow = size;
	assert(m_grow == 0 && grow > 0);
	m_grow = grow;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, std::uint64_t N, std::uint64_t G, size_t Cache>
inline void
data_pool<T, N, G, Cache>::clear(void)
{
	allocator_t* _alloc = allocator_wrap_t::get_allocator();
	m_main_trunk.clear();
	for (auto it = m_chunks.begin(); it != m_chunks.end(); ++it)
	{
		(*it)->clear();
		_alloc->free(*it);
		*it = nullptr;
	}
	m_chunks.clear();
	m_alloc_chunk = main_trunk;
	m_dealloc_chunk = main_trunk;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, std::uint64_t N, std::uint64_t G, size_t Cache>
inline auto
data_pool<T, N, G, Cache>::npos(T* p, size_t& n)->size_type
{
	if(m_dealloc_chunk == main_trunk)
		return base_t::_zero;

	n = m_dealloc_chunk;
	size_type pos = m_chunks[n]->npos(p);
	if (pos != base_t::_zero)
		return pos;

	size_t total = m_chunks.size();
	size_t m = n;
	for (size_t i = 1; i <= total / 2; ++i)
	{
		n = m_dealloc_chunk + i;
		if (n >= total) n -= total;
		pos = m_chunks[n]->npos(p);
		if (pos != base_t::_zero)
			return pos;

		m = n;
		n = m_dealloc_chunk >= i ? m_dealloc_chunk - i : total + m_dealloc_chunk - i;
		if (m == n) break;

		pos = m_chunks[n]->npos(p);
		if (pos != base_t::_zero)
			return pos;
	}

	return base_t::_zero;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, std::uint64_t N, std::uint64_t G, size_t Cache>
inline T*
data_pool<T, N, G, Cache>::malloc(void)
{
	T* p = m_main_trunk.malloc();
	if (p) return p;

	if (m_alloc_chunk < main_trunk)
	{
		p = m_chunks[m_alloc_chunk]->malloc();
		if (p) return p;
	}
	
	m_alloc_chunk = m_chunks.size();
	if (m_alloc_chunk == 0)
		m_dealloc_chunk = 0;
	else
		for (size_t i = 0; i < m_alloc_chunk; ++i)
		{
			p = m_chunks[i]->malloc();
			if (!p) continue;

			m_alloc_chunk = i;
			return p;
		}

	m_chunks.push_back(allocator_wrap_t::get_allocator()->malloc(m_grow));
	return m_chunks[m_alloc_chunk]->malloc();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, std::uint64_t N, std::uint64_t G, size_t Cache>
inline bool
data_pool<T, N, G, Cache>::free(T* p)
{
	if (m_main_trunk._free(p))
		return true;

	size_t n;
	size_type _pos = npos(p, n);
	assert(_pos != base_t::_zero);
	if (_pos == base_t::_zero)
		return false;

	m_chunks[n]->free(p);
	m_dealloc_chunk = n;

	if (m_chunks[n]->used() > 0)
		return true;

	n = m_chunks.size() - 1;
	if (n == m_dealloc_chunk)
		return true;

	if (m_chunks[n]->used() == 0)
	{
		allocator_wrap_t::get_allocator()->free(m_chunks[n]);
		m_chunks.pop_back();
		if (m_alloc_chunk == n)
			m_alloc_chunk = 0;
		--n;
	}

	base_t* _chunk = m_chunks[n];
	m_chunks[n] = m_chunks[m_dealloc_chunk];
	m_chunks[m_dealloc_chunk] = _chunk;

	if (m_alloc_chunk == n)
		m_alloc_chunk = m_dealloc_chunk;

	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}// namespace mem
////////////////////////////////////////////////////////////////////////////////////////////////////