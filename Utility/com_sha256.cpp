/**
* @file com_sha256.cpp
*
* @author Hourui (liquidmonkey)
**/

#include "Utility/com_sha256.hpp"

namespace Utility
{
////////////////////////////////////////////////////////////////////////////////
namespace com
{
////////////////////////////////////////////////////////////////////////////////
inline std::uint32_t endian(const std::uint32_t& data) {
    return
        ((data & 0xFF000000) >> 24)|
        ((data & 0x00FF0000) >> 8) |
        ((data & 0x0000FF00) << 8) |
        ((data & 0x000000FF) << 24);
}

inline std::uint64_t endian(const std::uint64_t data) {
    return
        ((data & 0x00000000000000FF) << 56)|
        ((data & 0x000000000000FF00) << 40)|
        ((data & 0x0000000000FF0000) << 24)|
        ((data & 0x00000000FF000000) << 8) |
        ((data & 0x000000FF00000000) >> 8) |
        ((data & 0x0000FF0000000000) >> 24)|
        ((data & 0x00FF000000000000) >> 40)|
        ((data & 0xFF00000000000000) >> 56);
}

#define Ch(x,y,z) (z ^ ( x& (y ^ z)))
#define Maj(x,y,z) ((x & y) | (z & (x | y)))
#define Sigma0(x) ((x >> 2 | x << 30) ^ (x >> 13 | x << 19) ^ (x >> 22 | x << 10))
#define Sigma1(x) ((x >> 6 | x << 26) ^ (x >> 11 | x << 21) ^ (x >> 25 | x << 7))
#define TRANSFORM(a,b,c,d,e,f,g,h,k,w,t)		\
	t = h + Sigma1(e) + Ch(e, f, g) + k + (w);\
	h = t + Sigma0(a) + Maj(a, b, c); d += t;
#define sigma0(x) ((x >> 7 | x << 25) ^ (x >> 18 | x << 14) ^ (x >> 3))
#define sigma1(x) ((x >> 17 | x << 15) ^ (x >> 19 | x << 13) ^ (x >> 10))
#define ROUND(a,b,c,d,e,f,g,h,chunk,w0,w1,w2,w3,w4,w5,w6,w7,w8,w9,w10,w11,w12,w13,w14,w15,t)\
    TRANSFORM(a, b, c, d, e, f, g, h, 0x428a2f98, w0 = endian(chunk[0]), t);\
    TRANSFORM(h, a, b, c, d, e, f, g, 0x71374491, w1 = endian(chunk[1]), t);\
    TRANSFORM(g, h, a, b, c, d, e, f, 0xb5c0fbcf, w2 = endian(chunk[2]), t);\
    TRANSFORM(f, g, h, a, b, c, d, e, 0xe9b5dba5, w3 = endian(chunk[3]), t);\
    TRANSFORM(e, f, g, h, a, b, c, d, 0x3956c25b, w4 = endian(chunk[4]), t);\
    TRANSFORM(d, e, f, g, h, a, b, c, 0x59f111f1, w5 = endian(chunk[5]), t);\
    TRANSFORM(c, d, e, f, g, h, a, b, 0x923f82a4, w6 = endian(chunk[6]), t);\
    TRANSFORM(b, c, d, e, f, g, h, a, 0xab1c5ed5, w7 = endian(chunk[7]), t);\
    TRANSFORM(a, b, c, d, e, f, g, h, 0xd807aa98, w8 = endian(chunk[8]), t);\
    TRANSFORM(h, a, b, c, d, e, f, g, 0x12835b01, w9 = endian(chunk[9]), t);\
    TRANSFORM(g, h, a, b, c, d, e, f, 0x243185be, w10 = endian(chunk[10]), t);\
    TRANSFORM(f, g, h, a, b, c, d, e, 0x550c7dc3, w11 = endian(chunk[11]), t);\
    TRANSFORM(e, f, g, h, a, b, c, d, 0x72be5d74, w12 = endian(chunk[12]), t);\
    TRANSFORM(d, e, f, g, h, a, b, c, 0x80deb1fe, w13 = endian(chunk[13]), t);\
    TRANSFORM(c, d, e, f, g, h, a, b, 0x9bdc06a7, w14 = endian(chunk[14]), t);\
    TRANSFORM(b, c, d, e, f, g, h, a, 0xc19bf174, w15 = endian(chunk[15]), t);\
    TRANSFORM(a, b, c, d, e, f, g, h, 0xe49b69c1, w0 += sigma1(w14) + w9 + sigma0(w1), t);\
    TRANSFORM(h, a, b, c, d, e, f, g, 0xefbe4786, w1 += sigma1(w15) + w10 + sigma0(w2), t);\
    TRANSFORM(g, h, a, b, c, d, e, f, 0x0fc19dc6, w2 += sigma1(w0) + w11 + sigma0(w3), t);\
    TRANSFORM(f, g, h, a, b, c, d, e, 0x240ca1cc, w3 += sigma1(w1) + w12 + sigma0(w4), t);\
    TRANSFORM(e, f, g, h, a, b, c, d, 0x2de92c6f, w4 += sigma1(w2) + w13 + sigma0(w5), t);\
    TRANSFORM(d, e, f, g, h, a, b, c, 0x4a7484aa, w5 += sigma1(w3) + w14 + sigma0(w6), t);\
    TRANSFORM(c, d, e, f, g, h, a, b, 0x5cb0a9dc, w6 += sigma1(w4) + w15 + sigma0(w7), t);\
    TRANSFORM(b, c, d, e, f, g, h, a, 0x76f988da, w7 += sigma1(w5) + w0 + sigma0(w8), t);\
    TRANSFORM(a, b, c, d, e, f, g, h, 0x983e5152, w8 += sigma1(w6) + w1 + sigma0(w9), t);\
    TRANSFORM(h, a, b, c, d, e, f, g, 0xa831c66d, w9 += sigma1(w7) + w2 + sigma0(w10), t);\
    TRANSFORM(g, h, a, b, c, d, e, f, 0xb00327c8, w10 += sigma1(w8) + w3 + sigma0(w11), t);\
    TRANSFORM(f, g, h, a, b, c, d, e, 0xbf597fc7, w11 += sigma1(w9) + w4 + sigma0(w12), t);\
    TRANSFORM(e, f, g, h, a, b, c, d, 0xc6e00bf3, w12 += sigma1(w10) + w5 + sigma0(w13), t);\
    TRANSFORM(d, e, f, g, h, a, b, c, 0xd5a79147, w13 += sigma1(w11) + w6 + sigma0(w14), t);\
    TRANSFORM(c, d, e, f, g, h, a, b, 0x06ca6351, w14 += sigma1(w12) + w7 + sigma0(w15), t);\
    TRANSFORM(b, c, d, e, f, g, h, a, 0x14292967, w15 += sigma1(w13) + w8 + sigma0(w0), t);\
    TRANSFORM(a, b, c, d, e, f, g, h, 0x27b70a85, w0 += sigma1(w14) + w9 + sigma0(w1), t);\
    TRANSFORM(h, a, b, c, d, e, f, g, 0x2e1b2138, w1 += sigma1(w15) + w10 + sigma0(w2), t);\
    TRANSFORM(g, h, a, b, c, d, e, f, 0x4d2c6dfc, w2 += sigma1(w0) + w11 + sigma0(w3), t);\
    TRANSFORM(f, g, h, a, b, c, d, e, 0x53380d13, w3 += sigma1(w1) + w12 + sigma0(w4), t);\
    TRANSFORM(e, f, g, h, a, b, c, d, 0x650a7354, w4 += sigma1(w2) + w13 + sigma0(w5), t);\
    TRANSFORM(d, e, f, g, h, a, b, c, 0x766a0abb, w5 += sigma1(w3) + w14 + sigma0(w6), t);\
    TRANSFORM(c, d, e, f, g, h, a, b, 0x81c2c92e, w6 += sigma1(w4) + w15 + sigma0(w7), t);\
    TRANSFORM(b, c, d, e, f, g, h, a, 0x92722c85, w7 += sigma1(w5) + w0 + sigma0(w8), t);\
    TRANSFORM(a, b, c, d, e, f, g, h, 0xa2bfe8a1, w8 += sigma1(w6) + w1 + sigma0(w9), t);\
    TRANSFORM(h, a, b, c, d, e, f, g, 0xa81a664b, w9 += sigma1(w7) + w2 + sigma0(w10), t);\
    TRANSFORM(g, h, a, b, c, d, e, f, 0xc24b8b70, w10 += sigma1(w8) + w3 + sigma0(w11), t);\
    TRANSFORM(f, g, h, a, b, c, d, e, 0xc76c51a3, w11 += sigma1(w9) + w4 + sigma0(w12), t);\
    TRANSFORM(e, f, g, h, a, b, c, d, 0xd192e819, w12 += sigma1(w10) + w5 + sigma0(w13), t);\
    TRANSFORM(d, e, f, g, h, a, b, c, 0xd6990624, w13 += sigma1(w11) + w6 + sigma0(w14), t);\
    TRANSFORM(c, d, e, f, g, h, a, b, 0xf40e3585, w14 += sigma1(w12) + w7 + sigma0(w15), t);\
    TRANSFORM(b, c, d, e, f, g, h, a, 0x106aa070, w15 += sigma1(w13) + w8 + sigma0(w0), t);\
    TRANSFORM(a, b, c, d, e, f, g, h, 0x19a4c116, w0 += sigma1(w14) + w9 + sigma0(w1), t);\
    TRANSFORM(h, a, b, c, d, e, f, g, 0x1e376c08, w1 += sigma1(w15) + w10 + sigma0(w2), t);\
    TRANSFORM(g, h, a, b, c, d, e, f, 0x2748774c, w2 += sigma1(w0) + w11 + sigma0(w3), t);\
    TRANSFORM(f, g, h, a, b, c, d, e, 0x34b0bcb5, w3 += sigma1(w1) + w12 + sigma0(w4), t);\
    TRANSFORM(e, f, g, h, a, b, c, d, 0x391c0cb3, w4 += sigma1(w2) + w13 + sigma0(w5), t);\
    TRANSFORM(d, e, f, g, h, a, b, c, 0x4ed8aa4a, w5 += sigma1(w3) + w14 + sigma0(w6), t);\
    TRANSFORM(c, d, e, f, g, h, a, b, 0x5b9cca4f, w6 += sigma1(w4) + w15 + sigma0(w7), t);\
    TRANSFORM(b, c, d, e, f, g, h, a, 0x682e6ff3, w7 += sigma1(w5) + w0 + sigma0(w8), t);\
    TRANSFORM(a, b, c, d, e, f, g, h, 0x748f82ee, w8 += sigma1(w6) + w1 + sigma0(w9), t);\
    TRANSFORM(h, a, b, c, d, e, f, g, 0x78a5636f, w9 += sigma1(w7) + w2 + sigma0(w10), t);\
    TRANSFORM(g, h, a, b, c, d, e, f, 0x84c87814, w10 += sigma1(w8) + w3 + sigma0(w11), t);\
    TRANSFORM(f, g, h, a, b, c, d, e, 0x8cc70208, w11 += sigma1(w9) + w4 + sigma0(w12), t);\
    TRANSFORM(e, f, g, h, a, b, c, d, 0x90befffa, w12 += sigma1(w10) + w5 + sigma0(w13), t);\
    TRANSFORM(d, e, f, g, h, a, b, c, 0xa4506ceb, w13 += sigma1(w11) + w6 + sigma0(w14), t);\
    TRANSFORM(c, d, e, f, g, h, a, b, 0xbef9a3f7, w14 + sigma1(w12) + w7 + sigma0(w15), t);\
    TRANSFORM(b, c, d, e, f, g, h, a, 0xc67178f2, w15 + sigma1(w13) + w8 + sigma0(w0), t);
////////////////////////////////////////////////////////////////////////////////
sha256::sha256(void) { reset(); }
sha256::sha256(const void* input, std::size_t length) { reset(); update(input, length); }

void sha256::reset(void) {
    m_finished = false;
    m_size = 0;
    m_pos = 0;
    m_result[0] = 0;
    memcpy(m_digest, init_data, sizeof(m_digest));
}

void sha256::update(const void* input, std::size_t length) {
    if (m_finished) {
        m_finished = false;
        memcpy(m_digest, m_state, sizeof(m_digest));
    }
    const char* p = (const char*)input;
    std::size_t tmp;
    if (m_size && m_pos) {
        tmp = BLOCK_SIZE - m_pos;
        if (tmp > length) {
            memcpy(m_tail + m_pos, input, length);
            m_pos += length;
            m_size += length;
            return;
        }

        memcpy(m_tail + m_pos, input, tmp);
        transform((std::uint32_t*)m_tail, 1);
        memset(m_tail, 0, BLOCK_SIZE);
        m_pos = 0;
        m_size += tmp;
        length -= tmp;
        p += tmp;
    }
    m_size += length;
    tmp = length >> 6;
    if (tmp) transform((std::uint32_t*)p, tmp);
    p += (tmp << 6);
    m_pos = length & 0x3F;
    memcpy(m_tail, p, m_pos);
}

void sha256::final(void) {
    memcpy(m_state, m_digest, sizeof(m_digest));
    char* p = m_tail + m_pos;
    *p = char(0x80);
    std::size_t tmp = m_pos <= DEAD_LINE ? DEAD_LINE - m_pos : BLOCK_SIZE + DEAD_LINE - m_pos;
    if (tmp) {
        memset(++p, 0, tmp);
        p += tmp;
    }
    *(std::uint64_t*)p = endian(std::uint64_t(m_size * 8));
    tmp = p > m_tail + BLOCK_SIZE ? 2 : 1;
    transform((std::uint32_t*)m_tail, tmp);
}

void sha256::transform(const std::uint32_t* chunk, size_t blocks) {
    std::uint32_t *A = m_digest + 0, *B = m_digest + 1, *C = m_digest + 2, *D = m_digest + 3, 
                  *E = m_digest + 4, *F = m_digest + 5, *G = m_digest + 6, *H = m_digest + 7;
    std::uint32_t w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10, w11, w12, w13, w14, w15,
                  t, a, b, c, d, e, f, g, h;
    while (blocks--) {
        a = *A, b = *B, c = *C, d = *D, e = *E, f = *F, g = *G, h = *H;
        ROUND(a, b, c, d, e, f, g, h, chunk, w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10, w11, w12, w13, w14, w15, t)
        (*A) += a,(*B) += b,(*C) += c,(*D) += d, (*E) += e, (*F) += f, (*G) += g, (*H) += h;
        chunk += 16;
    }
}
////////////////////////////////////////////////////////////////////////////////
} //!namespace com
////////////////////////////////////////////////////////////////////////////////
} //!namespace Utility