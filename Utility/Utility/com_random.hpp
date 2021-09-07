/**
* @file com_random.hpp
*
* @author Hourui (liquidmonkey)
**/

#ifndef __COM_RANDOM_HPP__
#define __COM_RANDOM_HPP__

#include <time.h>
#include <atomic>
#include <random>

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////
namespace com
{
////////////////////////////////////////////////////////////////////////////////
inline int rand(time_t seed = 0)
{
	static time_t m_nSeed = seed ? seed : time(nullptr);

	m_nSeed = m_nSeed * 214013L + 2531011L;

	return static_cast<int>((m_nSeed >> 16) & 0x7FFFFFFF);
}
////////////////////////////////////////////////////////////////////////////////
class random_lite
{
public:
	random_lite(time_t seed = 0) { set_seed(seed ? seed : time(nullptr)); }
	int next(void)
	{
		m_seed = m_seed * 214013L + 2531011L;
		return static_cast<int>(((m_seed) >> 16) & 0x7FFFFFFF);
	}

	void set_seed(time_t seed) { m_seed = seed; }
private:
	std::atomic<time_t> m_seed;
};
////////////////////////////////////////////////////////////////////////////////
template<typename T>
class rand_t
{
	friend class random;
	static_assert(std::is_arithmetic<T>::value, "T must be arithmetic!");
	using uniform = typename std::conditional<std::is_integral<T>::value, std::uniform_int_distribution<T>, std::uniform_real_distribution<T>>::type;
	static constexpr T MAX = std::is_integral<T>::value ? (std::numeric_limits<T>::max)() : T{ 1 };
public:
	inline T gen(void) { return m_dis(*m_gen); }
private:
	rand_t(std::mt19937* _gen, T _Min, T _Max) :m_gen(_gen), m_dis(_Min, _Max) {}
private:
	std::mt19937* m_gen = nullptr;
	uniform m_dis;
};

class random
{
	using seed_t = std::mt19937::result_type;
public:
	inline void set_seed(seed_t seed) { m_gen.seed(seed); }

	template<class T>
	inline rand_t<T> make(T _Min, T _Max) { assert(_Min < _Max); return{ &m_gen,_Min,_Max }; }

	template<class T>
	inline rand_t<T> make(T _Max = rand_t<T>::MAX) { return make<T>(0, _Max); }

	template<class T>
	inline T gen() { static rand_t<T>::uniform dis; return dis(m_gen); }

	template<class T>
	inline T gen(T _Min,T _Max) { assert(_Min < _Max); return rand_t<T>::uniform(_Min,_Max)(m_gen); }

	template<class T>
	inline T gen(T _Max) { return gen<T>(0,_Max); }
private:
	std::mt19937 m_gen{std::random_device()()};
};
////////////////////////////////////////////////////////////////////////////////
}// namespace com 
////////////////////////////////////////////////////////////////////////////////
}// namespace Utility
#endif //__COM_RANDOM_HPP__
