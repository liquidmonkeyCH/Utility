/**
* @file task_controler.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __TASK_CONTROLER_HPP__
#define __TASK_CONTROLER_HPP__

#include "task_dispatcher.hpp"
#include "logger.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace task
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class controler
{
public:
	friend class dispatcher;
	controler(void) = default;
	~controler(void) = default;

	controler(const controler&) = delete;
	controler& operator=(const controler&) = delete;

	inline void init(dispatcher* _dispatcher) {
		if (m_dispatcher)
			Clog::error_throw(errors::logic, "controler initialized!");
		m_dispatcher = _dispatcher;
	}
	inline virtual void post_request(object_iface* obj) { m_dispatcher->dispatch({ obj }); };
protected:
	dispatcher* m_dispatcher = nullptr;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace task
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__TASK_CONTROLER_HPP__