/**
* @file com_hex_caster.hpp
*
* @author Hourui (liquidmonkey)
**/

#ifndef __COM_HEX_CASTER_HPP__
#define __COM_HEX_CASTER_HPP__

#include <string.h>
#include <cstdint>
#include <cstddef>
#include <cctype>

namespace Utility
{
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace _impl
{
///////////////////////////////////////////////////////////////////////////////////////////////////
template<bool upper> struct bin_to_hex_define{ static constexpr char HEX[] = "0123456789abcdef"; };
template<> struct bin_to_hex_define<true> { static constexpr char HEX[] = "0123456789ABCDEF"; };
///////////////////////////////////////////////////////////////////////////////////////////////////
template<bool upper>
inline
const char* bin_to_hex(char* out, const std::uint8_t* data, size_t size)
{
	std::int64_t i = -1;
	for (const std::uint8_t* end = data + size; data != end; ++data)
	{
		out[++i] = bin_to_hex_define<upper>::HEX[*data >> 4];
		out[++i] = bin_to_hex_define<upper>::HEX[*data & 0x0F];
	}

	return out;
}

#define UTILITY_COM_HEX_TRANS(hex) ((hex & 0x40) ? (0x09 + (hex & 0x0F)) : (hex & 0x0F))
inline
void hex_to_bin(char* data, const char* hex, size_t len)
{
	for (size_t i = 0; i < len; i+=2,++data){
		*data = UTILITY_COM_HEX_TRANS(hex[i]) << 4 | UTILITY_COM_HEX_TRANS(hex[i + 1]);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<size_t size,bool hex_header>
class bin_to_hex_buffer
{
public:
	static constexpr size_t len = size * 2;
	bin_to_hex_buffer(void){ m_data[0] = 0x0; m_head = m_data; }
protected:
	char m_data[len + 1]{0};
	char* m_head;
};

template<size_t size>
class bin_to_hex_buffer<size,true>
{
public:
	static constexpr size_t len = size * 2 + 2;
	bin_to_hex_buffer(void){ memcpy(m_data, "0x", 3);  m_head = m_data+2; }
protected:
	char m_data[len + 1] {0};
	char* m_head;
};
///////////////////////////////////////////////////////////////////////////////////////////////////
}// namespace _impl
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace com
{
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class T, bool hex_header = false, bool upper = true>
struct hex_caster : public _impl::bin_to_hex_buffer<sizeof(T), hex_header>
{
	// bin -> hex
	hex_caster(const T& data) 
	{ _impl::bin_to_hex<upper>(this->m_head, (const std::uint8_t*)&data, sizeof(T)); }
	inline const char* c_str(void) { return this->m_data; }
};
///////////////////////////////////////////////////////////////////////////////////////////////////
template<bool upper = true>
inline const char* bin2hex(const void* data, size_t size, char* buf, size_t len, bool hex_header = false){
	if (len < 2) return nullptr;
	memset(buf, 0, len);
	if (hex_header) {
		len -= 2;
		memcpy(buf, "0x", 2);
		buf += 2;
	}
	len >>= 1;
	if (size > len) size = len;
	_impl::bin_to_hex<upper>(buf, (std::uint8_t*)data, size);
	return buf;
}

template<bool upper = true, class T>
inline const char* bin2hex(const T& data, char* buf, size_t len, bool hex_header = false) {
	return bin2hex<upper>(&data, sizeof(T), buf, len, hex_header);
}

template<bool hex_header = false, bool upper = true, class T>
inline hex_caster<T, hex_header, upper>
bin2hex(const T& data) { return hex_caster<T, hex_header, upper>(data); }
///////////////////////////////////////////////////////////////////////////////////////////////////
inline void hex2bin(const char* hex, void* data, size_t size, bool hex_header = false){
	size_t len = strlen(hex);
	if (hex_header) {
		len -= 2;
		hex += 2;
	}
	memset(data, 0, size);
	size *= 2;
	if (len > size) len = size;
	len = len > size ? len : size;
	_impl::hex_to_bin((char*)data, hex, len);
}

template<class T>
inline void hex2bin(const char* hex, T& data, bool hex_header = false) { hex2bin(hex,&data,sizeof(data),hex_header); }
#undef UTILITY_COM_HEX_TRANS
///////////////////////////////////////////////////////////////////////////////////////////////////
}// namespace com 
///////////////////////////////////////////////////////////////////////////////////////////////////
}// namespace Utility
#endif //__COM_HEX_CASTER_HPP__

