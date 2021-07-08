/**
* @file msg_pares_len.hpp
*
* @author Hourui (liquidmonkey)
*/
#ifndef __MSG_PARES_LEN_HPP__
#define __MSG_PARES_LEN_HPP__

#include "msg_message.hpp"
#include <cstdint>
#include <limits>

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace msg
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace __impl
{
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace __helper
{
template<typename len_t, bool need_change = false>
inline len_t endian(len_t* src) { return *src; }
template<>
inline std::uint64_t endian<std::uint64_t, true>(std::uint64_t* src) {
	*src =
		(((*(std::uint64_t*)src) & 0x00000000000000FF) << 56)|
		(((*(std::uint64_t*)src) & 0x000000000000FF00) << 40)|
		(((*(std::uint64_t*)src) & 0x0000000000FF0000) << 24)|
		(((*(std::uint64_t*)src) & 0x00000000FF000000) << 8) |
		(((*(std::uint64_t*)src) & 0x000000FF00000000) >> 8) |
		(((*(std::uint64_t*)src) & 0x0000FF0000000000) >> 24)|
		(((*(std::uint64_t*)src) & 0x00FF000000000000) >> 40)|
		(((*(std::uint64_t*)src) & 0xFF00000000000000) >> 56);
	return *src;
}
template<>
inline std::int64_t endian<std::int64_t, true>(std::int64_t* src) { return endian<std::uint64_t, true>((std::uint64_t*)src); }
template<>
inline std::uint32_t endian<std::uint32_t, true>(std::uint32_t* src) {
	*src =
		(((*(std::uint32_t*)src) & 0x000000FF) << 24)|
		(((*(std::uint32_t*)src) & 0x0000FF00) << 8) |
		(((*(std::uint32_t*)src) & 0x00FF0000) >> 8) |
		(((*(std::uint32_t*)src) & 0xFF000000) >> 24);
	return *src;
}
template<>
inline std::int32_t endian<std::int32_t, true>(std::int32_t* src) { return endian<std::uint32_t, true>((std::uint32_t*)src); }
template<>
inline std::uint16_t endian<std::uint16_t, true>(std::uint16_t* src) {
	*src =
		(((*(std::uint16_t*)src) & 0x00FF) << 8)|
		(((*(std::uint16_t*)src) & 0xFF00) >> 8);
	return *src;
}
template<>
inline std::int16_t endian<std::int16_t, true>(std::int16_t* src) { return endian<std::uint16_t, true>((std::uint16_t*)src); }
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename len_t, class msg_buffer, bool include_sizeof_len = true, bool need_endian = false>
class comfirmer : public message<msg_buffer>
{
public:
	using base = message<msg_buffer>;
	static constexpr std::size_t max = (std::numeric_limits<net_size_t>::max)() > (std::numeric_limits<len_t>::max)() ? (std::numeric_limits<len_t>::max)() : (std::numeric_limits<net_size_t>::max)();
	static constexpr net_size_t type_size = sizeof(len_t);
	comfirmer(void)
	{
		static_assert(std::is_integral<len_t>::value, "len_t mast be integral!");
		static_assert(base::max_message_len > type_size &&
			base::max_message_len <= max, "max_message_len out of range!");
	}
	virtual ~comfirmer(void) = default;
	template<class message_wrap, class handler_manager> friend class msg::controler;
private:
	state comfirm(void)
	{
		net_size_t size = this->m_size;
		if (size == 0)							// 未获取消息长度
		{
			size = type_size;
			if (!this->readable_state(size))	// 消息已无效
				return state::bad;

			if (size == 0)						// 消息长度不足
				return state::pending;

			len_t len = 0;
			if(!this->get(len))
				return state::error;
			
			this->m_size = __helper::endian<len_t, need_endian>(&len);
			if (len < 0 || len > max)
				return state::error;

			if (!include_sizeof_len) this->m_size += type_size;

			if (this->m_size < type_size || this->m_size > base::max_message_len)
				return state::error;

			size = this->m_size;
		}

		if (!this->readable_state(size))	// 消息已无效
			return state::bad;

		//  last_read == 0
		if (size == 0)						// 消息长度不足
			return state::pending;

		this->set_read_limit(this->m_size);	// 消息完整 
		return state::ok;
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}// !__impl
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace pares_len
{
////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename len_t, class msg_buffer, bool include_sizeof_len = true, bool need_endian = false>
using comfirmer = __impl::comfirmer<len_t, msg_buffer, include_sizeof_len, need_endian>;
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class msg_buffer, bool include_sizeof_len = true>
using comfirmer64 = comfirmer<std::uint64_t, msg_buffer, include_sizeof_len>;
template<class msg_buffer, bool include_sizeof_len = true>
using comfirmer32 = comfirmer<std::uint32_t, msg_buffer, include_sizeof_len>;
template<class msg_buffer, bool include_sizeof_len = true>
using comfirmer16 = comfirmer<std::uint16_t, msg_buffer, include_sizeof_len>;
template<class msg_buffer, bool include_sizeof_len = true>
using comfirmer8 = comfirmer<std::uint8_t, msg_buffer, include_sizeof_len>;
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace reverse 
{
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class buffer_type, bool include_sizeof_len = true>
using comfirmer64 = comfirmer<std::uint64_t, buffer_type, include_sizeof_len, true>;
template<class buffer_type, bool include_sizeof_len = true>
using comfirmer32 = comfirmer<std::uint32_t, buffer_type, include_sizeof_len, true>;
template<class buffer_type, bool include_sizeof_len = true>
using comfirmer16 = comfirmer<std::uint16_t, buffer_type, include_sizeof_len, true>;
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace reverse
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace pares_len
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace msg
////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace Utility 
#endif //__MSG_MESSAGE_HPP__
