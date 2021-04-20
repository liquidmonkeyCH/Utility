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

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace task
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class controler;
class object_iface
{
public:
	struct task_info
	{
		object_iface* m_obj;
		inline void exec(void) { m_obj->exec(); }
	};
	friend struct task_info;

	object_iface(void) = default;
	virtual ~object_iface(void) = default;

	inline controler* get_controler(void) const { return m_controler; }
protected:
	virtual void exec(void) = 0;
protected:
	controler* m_controler = nullptr;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class object : public object_iface
{
	using task_t = std::function<void(void)>;
public:
	object(void) = default;
	virtual ~object(void) = default;
	object(const object&) = delete;
	object& operator=(const object&) = delete;

	void init(controler* _controler);

	template<class F, class... Args>
	inline void schedule(F&& f, Args&&... args){
		post(task_t(std::bind(std::forward<F>(f),std::forward<Args>(args)...)));
	}

	void close(void);
protected:
	void post(task_t&& task);
	virtual void exec(void);
	virtual void on_close(void) {}
protected:
	std::mutex m_mutex;
	bool m_good = false;
	std::queue<task_t> tasks;
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace task
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__TASK_OBJECT_HPP__