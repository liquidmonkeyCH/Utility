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
#include "task_channel.hpp"
#include "task_controler.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace task
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class object_iface : public channel_node
{
public:
	object_iface(void) : channel_node(false) {};
	virtual ~object_iface(void) = default;
protected:
	inline void post_request(void) { m_controler->post_request(this); }
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
}//namespace task
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__TASK_OBJECT_HPP__