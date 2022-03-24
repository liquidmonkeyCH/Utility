/**
* @file com_random.hpp
*
* @author Hourui (liquidmonkey)
**/

#ifndef __COM_RANDOM_HPP__
#define __COM_RANDOM_HPP__

#include <random>
#include <time.h>
#include <atomic>
#include "com_numeric.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////
namespace com
{
////////////////////////////////////////////////////////////////////////////////
namespace widgets
{
////////////////////////////////////////////////////////////////////////////////
inline int rand(time_t seed = 0)
{
	static time_t m_nSeed = seed ? seed : time(nullptr);

	m_nSeed = m_nSeed * 214013L + 2531011L;

	return static_cast<int>((m_nSeed >> 16) & 0x7FFFFFFF);
}
////////////////////////////////////////////////////////////////////////////////
class random
{
public:
	random(time_t seed = 0) { set_seed(seed ? seed : time(nullptr)); }
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
}//namespace widgets
////////////////////////////////////////////////////////////////////////////////
template<typename T>
class random_t
{
	friend class random;
	static_assert(std::is_arithmetic<T>::value, "T must be arithmetic!");
	static constexpr T MAX = std::is_integral<T>::value ? (std::numeric_limits<T>::max)() : T{ 1 };
public:
	using uniform = typename std::conditional<std::is_integral<T>::value, std::uniform_int_distribution<T>, std::uniform_real_distribution<T>>::type;
	inline T gen(void) { return m_dis(*m_gen); }
	T operator()(void) { return m_dis(*m_gen); }
private:
	random_t(std::mt19937* _gen, T _Min, T _Max) :m_gen(_gen), m_dis(_Min, _Max) {}
private:
	std::mt19937* m_gen = nullptr;
	uniform m_dis;
};
////////////////////////////////////////////////////////////////////////////////
class random
{
	using seed_t = std::mt19937::result_type;
public:
	inline void set_seed(seed_t seed) { m_gen.seed(seed); }

	template<class T>
	inline random_t<T> make(T _Min, T _Max) { auto r = std::minmax(_Min, _Max); return{ &m_gen, r.first, r.second }; }

	template<class T>
	inline random_t<T> make(T _Max = random_t<T>::MAX) { return make<T>(0, _Max); }

	template<class T>
	inline T gen() { static random_t<T>::uniform dis; return dis(m_gen); }

	template<class T>
	inline T gen(T _Min,T _Max) { auto r = std::minmax(_Min, _Max); return random_t<T>::uniform(r.first, r.second)(m_gen); }

	template<class T>
	inline T gen(T _Max) { return gen<T>(0,_Max); }
private:
	std::mt19937 m_gen{std::random_device()()};
};
////////////////////////////////////////////////////////////////////////////////
inline int rand(unsigned int seed = 0)
{
	static std::mt19937 m_gen{ seed ? seed : std::random_device()() };
	static std::uniform_int_distribution<int> dis;

	return dis(m_gen);
}
////////////////////////////////////////////////////////////////////////////////
template<class T>
inline T rand(unsigned int seed = 0)
{
	static std::mt19937 m_gen{ seed ? seed : std::random_device()() };
	static random_t<T>::uniform dis;

	return dis(m_gen);
}
////////////////////////////////////////////////////////////////////////////////
}// namespace com 
////////////////////////////////////////////////////////////////////////////////
}// namespace Utility
#endif //__COM_RANDOM_HPP__
