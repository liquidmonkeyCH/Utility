/**
* @file task_object.cpp
*
* @author Hourui (liquidmonkey)
*/

#include "Utility/task_object.hpp"
#include "Utility/task_controler.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace task
{
////////////////////////////////////////////////////////////////////////////////////////////////////
void object::init(controler* _controler) {
	std::lock_guard<std::mutex> lock(m_mutex);
	if (tasks.empty()) {
		m_controler = _controler;
		m_good = true;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void object::post(task_t&& task) {
	std::lock_guard<std::mutex> lock(m_mutex);
	if (!m_good) return;
	bool res = tasks.empty();
	tasks.push(std::move(task));
	if (res)
		m_controler->post_request(this);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void object::exec(void) {
	std::unique_lock<std::mutex> lock(m_mutex);
	if (!m_good) {
		lock.unlock();
		on_close();
		lock.lock();
		while (!tasks.empty())tasks.pop();
		return;
	}
	task_t task(std::move(tasks.front()));
	lock.unlock();
	task();
	lock.lock();
	tasks.pop();
	if(!tasks.empty())
		m_controler->post_request(this);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void object::close(void) {
	std::lock_guard<std::mutex> lock(m_mutex);
	if (!m_good) return;
	bool res = tasks.empty();
	m_good = false;
	if (res)
		m_controler->post_request(this);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace task
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 