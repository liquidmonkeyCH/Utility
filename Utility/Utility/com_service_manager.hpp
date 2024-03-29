/**
* @file com_service_manager.hpp
*
* @author hourui (liquidmonkey)
**/

#ifndef __COM_SERVICE_MANAGER_HPP__
#define __COM_SERVICE_MANAGER_HPP__

#include "logger.hpp"
#include <type_traits>
#include <functional>
#include "com_guard.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace com
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class ServiceManager;
namespace iface { class Service; }
namespace wrap { template<class> class Service; }
////////////////////////////////////////////////////////////////////////////////////////////////////
}
namespace _impl 
{
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
struct service_refence
{
	using param_t = typename std::conditional<std::is_base_of<com::iface::Service, T>::value, T, com::wrap::Service<T>>::type;

	service_refence(void) :_data(nullptr) {}
	~service_refence(void) { destory(); }

	service_refence(const service_refence&) = delete;
	service_refence& operator=(const service_refence&) = delete;
	service_refence(const service_refence&&) = delete;
	service_refence& operator=(const service_refence&&) = delete;
	friend class com::ServiceManager;
private:
	void create(void) { _data = new param_t; }
	void destory(void) { delete _data; _data = nullptr; }

	param_t* _data;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace _impl
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace com
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class ServiceManager
{
public:
	enum class error
	{
		not_found = 0,
		duplicate_attach,
		duplicate_detach,
	};
protected:
	template<typename T>
	using refence = _impl::service_refence<T>;

	template<typename T>
	static refence<T>* _refence()
	{
		static refence<T> res;
		return &res;
	}

	template<typename T>
	using param_t = typename refence<T>::param_t;
protected:
	template<typename T, typename... Args>
	struct _bind_function;

	template<typename T>
	struct _bind_function<T>
	{
		static void run(T* service) {}
	};

	template<class T, class F, typename... Args>
	struct _bind_function<T, F, Args...>
	{
		static auto run(T* service, F&& f, Args&& ... args)->decltype(std::bind(std::forward<F>(f), std::placeholders::_1, std::forward<Args>(args)...)((T*)(nullptr)))
		{
			return std::bind(std::forward<F>(f), std::placeholders::_1, std::forward<Args>(args)...)(service);
		}
	};
public:
	template<typename T>
	static auto GetService(void)->param_t<T>*
	{
		refence<T>* res = _refence<T>();
		if (res->_data)
			return res->_data;

		Clog::error("Service not found (%s)", param_t<T>::ID());
		return nullptr;
	}

	template<typename T, typename... Args>
	static auto Attach(Args&& ... args)->decltype(_bind_function<param_t<T>, Args...>::run((param_t<T>*)nullptr, std::forward<Args>(args)...))
	{
		refence<T>* res = _refence<T>();
		if (res->_data)
			Clog::error_throw(error::duplicate_attach, "Service duplicate attach (%s)", param_t<T>::ID());

		res->create();
		return _bind_function<param_t<T>, Args...>::run(res->_data, std::forward<Args>(args)...);
	}

	template<typename T, typename... Args>
	static auto Detach(Args&& ... args)->decltype(_bind_function<param_t<T>, Args...>::run((param_t<T>*)nullptr, std::forward<Args>(args)...))
	{
		refence<T>* res = _refence<T>();
		if (!res->_data)
			Clog::error_throw(error::duplicate_detach, "Service duplicate detach (%s)", param_t<T>::ID());

		guard<refence<T>*> _guard(res, &refence<T>::destory);
		return _bind_function<param_t<T>, Args...>::run(res->_data, std::forward<Args>(args)...);
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}// namespace com
////////////////////////////////////////////////////////////////////////////////////////////////////
}// namespace Utility
#endif // __COM_SERVICE_MANAGER_HPP__
