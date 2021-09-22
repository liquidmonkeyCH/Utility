/**
* @file task_object.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __TASK_OBJECT_HPP__
#define __TASK_OBJECT_HPP__

#include <mutex>
#include <queue>
#include <functional>
#include <assert.h>
#include "task_dispatcher.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace task
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace wrap
{
	template<class T>
	class object_channel : public T
	{
		static_assert(std::is_base_of<channel_node, T>::value, "T must Inherits from channel_node");
	public:
		object_channel(void){
			m_channel = dispatcher::malloc();
			assert(m_channel);
			channel_node::enter_channel(m_channel); 
		}
		virtual ~object_channel(void) { if(m_channel) dispatcher::free(m_channel);}
		object_channel(const object_channel&) = delete;
		object_channel& operator=(const object_channel&) = delete;
		inline channel* to_channel(void) { return m_channel; }
		inline virtual void enter_channel(channel* p_channel) { m_channel->enter_channel(p_channel); }
		inline virtual void leave_channel(void) { m_channel->leave_channel(); }
	private:
		channel* m_channel = nullptr;
	};
}
////////////////////////////////////////////////////////////////////////////////////////////////////
class object_iface : public channel_node
{
public:
	friend class controler;
	friend class dispatcher;
	object_iface(void) : channel_node(false) {};
	virtual ~object_iface(void) = default;
protected:
	inline void post_request(void) { m_controler->post_request(this); }
	inline bool do_dispatch(void) { return m_controler->dispatch_obj(this); }
protected:
	controler* m_controler = nullptr;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class object : public object_iface
{
	using task_t = std::function<void(void)>;
	friend class controler;
public:
	object(void) = default;
	virtual ~object(void) = default;
	object(const object&) = delete;
	object& operator=(const object&) = delete;

	void init(controler* _controler);

	template<class F, class... Args>
	inline void exec(F&& f, Args&&... args){
		post_call(task_t(std::bind(std::forward<F>(f),std::forward<Args>(args)...)));
	}

	void close(void);
protected:
	virtual void on_close(void) {}
private:
	void post_call(task_t&& task);
	bool exec_task(void);
private:
	std::mutex m_mutex;
	bool m_good = false;
	std::queue<task_t> tasks;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
using object_channel = wrap::object_channel<object>;
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace task
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__TASK_OBJECT_HPP__