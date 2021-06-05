/**
* @file com_aes.hpp
*
* @author Hourui (liquidmonkey)
**/

#ifndef __COM_AES_HPP__
#define __COM_AES_HPP__

#include <cstdint>
#include <string.h>

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////
namespace _impl
{
////////////////////////////////////////////////////////////////////////////////
class aes_base
{
public:
	enum class mode_t : int { ECB, CBC, CTR, CFB, OFB, GCM };
	enum class padding_t : int { None, Zero, Pkcs7, AnsiX923, iso10126  };
	enum errors_t : int { Success, NeedKey, NeedPadding, Overflow, BadLen, BadMode };
	static constexpr size_t BLOCK_WORD = 4;
	static constexpr size_t BLOCK_SIZE = 4 * BLOCK_WORD;
	static constexpr size_t SIZE_MASK = BLOCK_SIZE - 1;

	struct iv_t
	{
		char m_data[BLOCK_SIZE];
		std::uint8_t m_len = 0;
		operator char* () { return m_data; }
	};
public:
	aes_base(void) = default;
	virtual ~aes_base(void) = default;
public:
	void set_iv(const char* iv, size_t size = BLOCK_SIZE);
	iv_t get_iv(void){ if (m_iv.m_len == 0) gen_iv(); return m_iv; }
	void set_auth(const char* ptr, size_t len) { m_auth_ptr = ptr; m_auth_len = len; }	// for GCM MODE
protected:
	void gen_iv(void);
	void gen_key(const char* key, uint32_t* w, int Nk, int KeyLen, int KeyExt);
	int check_param(size_t& out_size, size_t& in_size, mode_t mod, padding_t& padding);
	int do_encrypt(char* out, size_t& out_size, const char* in, size_t in_size, const std::uint32_t* key, mode_t mod, padding_t padding, int Nr);
	int do_decrypt(char* out, size_t& out_size, const char* in, size_t in_size, const std::uint32_t* key, mode_t mod, padding_t padding, int Nr);
protected:
	iv_t m_iv;
	std::uint8_t m_end[BLOCK_SIZE] = {0};
	const char* m_auth_ptr = nullptr;
	size_t m_auth_len = 0;
};
////////////////////////////////////////////////////////////////////////////////
template<size_t Nk, size_t Nr>
struct aes : public aes_base
{
	static constexpr size_t KEY_LEN = 4 * Nk;
	static constexpr size_t KEY_EXT = 4 * (Nr + 1);
	void set_key(const char* key, size_t size = KEY_LEN) {
		memset(m_key, 0, KEY_LEN);
		size = size > KEY_LEN ? KEY_LEN : size;
		memcpy(m_key, key, size);
		gen_key(m_key, m_w, Nk, KEY_LEN, KEY_EXT);
		key_init = true;
	}

	inline int encrypt(char* out, size_t out_size, const char* in, size_t in_size, mode_t mode = mode_t::ECB, padding_t padding = padding_t::Pkcs7) {
		return key_init ? do_encrypt(out, out_size, in, in_size, m_w, mode, padding, Nr) : static_cast<int>(NeedKey);
	}

	inline int decrypt(char* out, size_t out_size, const char* in, size_t in_size, mode_t mode = mode_t::ECB, padding_t padding = padding_t::Pkcs7) {
		return key_init ? do_encrypt(out, out_size, in, in_size, m_w, mode, padding, Nr) : static_cast<int>(NeedKey);
	}

	inline int encrypt(size_t& out_size, char* out, const char* in, size_t in_size, mode_t mode = mode_t::ECB, padding_t padding = padding_t::Pkcs7) {
		return key_init ? do_encrypt(out, out_size, in, in_size, m_w, mode, padding, Nr) : static_cast<int>(NeedKey);
	}

	inline int decrypt(size_t& out_size, char* out, const char* in, size_t in_size, mode_t mode = mode_t::ECB, padding_t padding = padding_t::Pkcs7) {
		return key_init ? do_encrypt(out, out_size, in, in_size, m_w, mode, padding, Nr) : static_cast<int>(NeedKey);
	}
protected:
	inline void KeyExpansion(void) {

	}
protected:
	char m_key[KEY_LEN];
	bool key_init = false;
	uint32_t m_w[KEY_EXT];
};
////////////////////////////////////////////////////////////////////////////////
}// namespace _impl 
////////////////////////////////////////////////////////////////////////////////
namespace com
{
////////////////////////////////////////////////////////////////////////////////
using aes128 = _impl::aes<4, 10>;
using aes192 = _impl::aes<6, 12>;
using aes256 = _impl::aes<8, 14>;
////////////////////////////////////////////////////////////////////////////////
}
////////////////////////////////////////////////////////////////////////////////
}// namespace Utility
#endif //__COM_AES_HPP__
