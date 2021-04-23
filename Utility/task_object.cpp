/**
* @file task_object.cpp
*
* @author Hourui (liquidmonkey)
*/

#include "Utility/task_object.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace task
{
////////////////////////////////////////////////////////////////////////////////////////////////////
void object::init(controler* p_controler) {
	std::lock_guard<std::mutex> lock(m_mutex);
	if (!m_good && tasks.empty()) {
		m_controler = p_controler;
		m_good = true;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void object::post_call(task_t&& task) {
	bool need_post = false;{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (!m_good || !m_controler) return;
		need_post = tasks.empty();
		tasks.push(std::move(task));
	}
	if (need_post)
		post_request();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool object::exec_task(void) {
	std::unique_lock<std::mutex> lock(m_mutex);
	if (!m_good) {
		lock.unlock();
		leave_channel();
		on_close();
		lock.lock();
		while (!tasks.empty())tasks.pop();
		return false;
	}
	task_t task(std::move(tasks.front()));
	lock.unlock();
	task();
	lock.lock();
	tasks.pop();
	return tasks.empty() ? false : true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void object::close(void) {
	bool need_post = false; {
		std::lock_guard<std::mutex> lock(m_mutex);
		if (!m_good || !m_controler) return;
		need_post = tasks.empty();
		m_good = false;
	}
	if (need_post)
		post_request();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace task
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 