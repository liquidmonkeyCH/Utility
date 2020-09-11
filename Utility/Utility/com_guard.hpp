/**
* @file com_guard.hpp
*
* @author Hourui (liquidmonkey)
**/

#ifndef __COM_GUARD_HPP__
#define __COM_GUARD_HPP__

#include <functional>

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////
namespace com
{
////////////////////////////////////////////////////////////////////////////////
template<class T>
class guard
{
public:
	using param_t = typename std::conditional<std::is_reference<T>::value || std::is_pointer<T>::value, T, T&>::type;
	using func_t = std::function<void(param_t)>;
	guard(param_t param, func_t&& destory) :m_param(param), m_destory(std::move(destory)) {}
	guard(param_t param, func_t&& create, func_t&& destory)
		: m_param(param), m_destory(std::move(destory)) {
		create(m_param);
	}
	~guard(void) { m_destory(m_param); }

	guard(void) = delete;
	guard(const guard&) = delete;
	guard(const guard&&) = delete;
	guard& operator=(const guard&) = delete;
	guard& operator=(const guard&&) = delete;
private:
	param_t m_param;
	func_t m_destory;
};
////////////////////////////////////////////////////////////////////////////////
}// namespace com 
////////////////////////////////////////////////////////////////////////////////
}// namespace Utility
#endif //__COM_GUARD_HPP__
