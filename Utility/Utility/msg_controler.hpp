/**
* @file msg_controler.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __MSG_CONTROLER_HPP__
#define __MSG_CONTROLER_HPP__

#include "task_controler.hpp"
#include "msg_message.hpp"
#include "msg_object.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace msg
{
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class message_wrap,class handler_manager>
class controler : public handler_manager,public task::controler
{
public:
	using message_t = message_wrap;
public:
	controler(void) = default;
	~controler(void) = default;

	controler(const controler&) = delete;
	controler& operator=(const controler&) = delete;
public:
	inline void post_request(task::channel_node* object)
	{
		object_iface* obj = dynamic_cast<object_iface*>(object);
		message_t* message = dynamic_cast<message_t*>(obj->get_message());
		switch (message->comfirm())
		{
		case state::ok:
		case state::bad:
			this->post_node(obj);
			break;
		case state::pending:
			break;
		case state::error:
			obj->handle_error(err::pre_unpack);
			break;
		default:
			break;
		}
	}
private:
	bool dispatch_obj(task::object_iface* object)
	{
		object_iface* obj = dynamic_cast<object_iface*>(object);
		mem::message* message = obj->get_message();
		message_t* msg = dynamic_cast<message_t*>(message);

		if (!msg->is_good()){
			obj->do_close();
			return false;
		}

		handler_t handle = this->get_handle(msg);
		int ret = err::null_handle;
		if (!handle || (ret = handle(obj, message)) != err::none)
		{
			obj->handle_error(ret);
			obj->do_close();
			return false;
		}

		msg->commit();
		switch (msg->comfirm())
		{
		case state::ok:
			return true;
		case state::pending:
			break;
		case state::error:
			obj->handle_error(err::pre_unpack);
			obj->do_close();
			break;
		case state::bad:
			obj->do_close();
			break;
		default:
			break;
		}

		return false;
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace task
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__MSG_CONTROLER_HPP__