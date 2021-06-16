/******************************************************************************
 *  @file         openssl_aes.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-16
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        
 *  mugglec crypt compatible with openssl AES
 *
 *  this source file are all copied and modified from openssl(Apache License v2.0)
 *****************************************************************************/
 
#include "openssl_aes.h"
#include <string.h>
#include "muggle/c/crypt/aes.h"
#include "muggle/c/base/err.h"
#include "muggle/c/log/log.h"

/*
 * Compatibility with openssl
 *
 * this source file are all copied and modified from openssl(Apache License v2.0)
 *
 * OpenSSL_1_1_1-stable, Enable with: ./config no-asm -DOPENSSL_AES_CONST_TIME
 *
 * */

typedef union openssl_union {
    unsigned char b[8];
    uint32_t w[2];
    uint64_t d;
} openssl_uni;

/*
 * Compute w := (w * x) mod (x^8 + x^4 + x^3 + x^1 + 1)
 * Therefore the name "xtime".
 */
static void openssl_xtime_u32(uint32_t *w)
{
    uint32_t a, b;

    a = *w;
    b = a & 0x80808080u;
    a ^= b;
    b -= b >> 7;
    b &= 0x1B1B1B1Bu;
    b ^= a << 1;
    *w = b;
}

static void openssl_xtime_u64(uint64_t *w)
{
    uint64_t a, b;

    a = *w;
    b = a & 0x8080808080808080uLL;
    a ^= b;
    b -= b >> 7;
    b &= 0x1B1B1B1B1B1B1B1BuLL;
    b ^= a << 1;
    *w = b;
}

/*
 * This computes w := S * w ^ -1 + c, where c = {01100011}.
 * Instead of using GF(2^8) mod (x^8+x^4+x^3+x+1} we do the inversion
 * in GF(GF(GF(2^2)^2)^2) mod (X^2+X+8)
 * and GF(GF(2^2)^2) mod (X^2+X+2)
 * and GF(2^2) mod (X^2+X+1)
 * The first part of the algorithm below transfers the coordinates
 * {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80} =>
 * {1,Y,Y^2,Y^3,Y^4,Y^5,Y^6,Y^7} with Y=0x41:
 * {0x01,0x41,0x66,0x6c,0x56,0x9a,0x58,0xc4}
 * The last part undoes the coordinate transfer and the final affine
 * transformation S:
 * b[i] = b[i] + b[(i+4)%8] + b[(i+5)%8] + b[(i+6)%8] + b[(i+7)%8] + c[i]
 * in one step.
 * The multiplication in GF(2^2^2^2) is done in ordinary coords:
 * A = (a0*1 + a1*x^4)
 * B = (b0*1 + b1*x^4)
 * AB = ((a0*b0 + 8*a1*b1)*1 + (a1*b0 + (a0+a1)*b1)*x^4)
 * When A = (a0,a1) is given we want to solve AB = 1:
 * (a) 1 = a0*b0 + 8*a1*b1
 * (b) 0 = a1*b0 + (a0+a1)*b1
 * => multiply (a) by a1 and (b) by a0
 * (c) a1 = a1*a0*b0 + (8*a1*a1)*b1
 * (d) 0 = a1*a0*b0 + (a0*a0+a1*a0)*b1
 * => add (c) + (d)
 * (e) a1 = (a0*a0 + a1*a0 + 8*a1*a1)*b1
 * => therefore
 * b1 = (a0*a0 + a1*a0 + 8*a1*a1)^-1 * a1
 * => and adding (a1*b0) to (b) we get
 * (f) a1*b0 = (a0+a1)*b1
 * => therefore
 * b0 = (a0*a0 + a1*a0 + 8*a1*a1)^-1 * (a0+a1)
 * Note this formula also works for the case
 * (a0+a1)*a0 + 8*a1*a1 = 0
 * if the inverse element for 0^-1 is mapped to 0.
 * Repeat the same for GF(2^2^2) and GF(2^2).
 * We get the following algorithm:
 * inv8(a0,a1):
 *   x0 = a0^a1
 *   [y0,y1] = mul4([x0,a1],[a0,a1]); (*)
 *   y1 = mul4(8,y1);
 *   t = inv4(y0^y1);
 *   [b0,b1] = mul4([x0,a1],[t,t]); (*)
 *   return [b0,b1];
 * The non-linear multiplies (*) can be done in parallel at no extra cost.
 */
static void openssl_sub_u32(uint32_t *w)
{
    uint32_t x, y, a1, a2, a3, a4, a5, a6;

    x = *w;
    y = ((x & 0xFEFEFEFEu) >> 1) | ((x & 0x01010101u) << 7);
    x &= 0xDDDDDDDDu;
    x ^= y & 0x57575757u;
    y = ((y & 0xFEFEFEFEu) >> 1) | ((y & 0x01010101u) << 7);
    x ^= y & 0x1C1C1C1Cu;
    y = ((y & 0xFEFEFEFEu) >> 1) | ((y & 0x01010101u) << 7);
    x ^= y & 0x4A4A4A4Au;
    y = ((y & 0xFEFEFEFEu) >> 1) | ((y & 0x01010101u) << 7);
    x ^= y & 0x42424242u;
    y = ((y & 0xFEFEFEFEu) >> 1) | ((y & 0x01010101u) << 7);
    x ^= y & 0x64646464u;
    y = ((y & 0xFEFEFEFEu) >> 1) | ((y & 0x01010101u) << 7);
    x ^= y & 0xE0E0E0E0u;
    a1 = x;
    a1 ^= (x & 0xF0F0F0F0u) >> 4;
    a2 = ((x & 0xCCCCCCCCu) >> 2) | ((x & 0x33333333u) << 2);
    a3 = x & a1;
    a3 ^= (a3 & 0xAAAAAAAAu) >> 1;
    a3 ^= (((x << 1) & a1) ^ ((a1 << 1) & x)) & 0xAAAAAAAAu;
    a4 = a2 & a1;
    a4 ^= (a4 & 0xAAAAAAAAu) >> 1;
    a4 ^= (((a2 << 1) & a1) ^ ((a1 << 1) & a2)) & 0xAAAAAAAAu;
    a5 = (a3 & 0xCCCCCCCCu) >> 2;
    a3 ^= ((a4 << 2) ^ a4) & 0xCCCCCCCCu;
    a4 = a5 & 0x22222222u;
    a4 |= a4 >> 1;
    a4 ^= (a5 << 1) & 0x22222222u;
    a3 ^= a4;
    a5 = a3 & 0xA0A0A0A0u;
    a5 |= a5 >> 1;
    a5 ^= (a3 << 1) & 0xA0A0A0A0u;
    a4 = a5 & 0xC0C0C0C0u;
    a6 = a4 >> 2;
    a4 ^= (a5 << 2) & 0xC0C0C0C0u;
    a5 = a6 & 0x20202020u;
    a5 |= a5 >> 1;
    a5 ^= (a6 << 1) & 0x20202020u;
    a4 |= a5;
    a3 ^= a4 >> 4;
    a3 &= 0x0F0F0F0Fu;
    a2 = a3;
    a2 ^= (a3 & 0x0C0C0C0Cu) >> 2;
    a4 = a3 & a2;
    a4 ^= (a4 & 0x0A0A0A0A0Au) >> 1;
    a4 ^= (((a3 << 1) & a2) ^ ((a2 << 1) & a3)) & 0x0A0A0A0Au;
    a5 = a4 & 0x08080808u;
    a5 |= a5 >> 1;
    a5 ^= (a4 << 1) & 0x08080808u;
    a4 ^= a5 >> 2;
    a4 &= 0x03030303u;
    a4 ^= (a4 & 0x02020202u) >> 1;
    a4 |= a4 << 2;
    a3 = a2 & a4;
    a3 ^= (a3 & 0x0A0A0A0Au) >> 1;
    a3 ^= (((a2 << 1) & a4) ^ ((a4 << 1) & a2)) & 0x0A0A0A0Au;
    a3 |= a3 << 4;
    a2 = ((a1 & 0xCCCCCCCCu) >> 2) | ((a1 & 0x33333333u) << 2);
    x = a1 & a3;
    x ^= (x & 0xAAAAAAAAu) >> 1;
    x ^= (((a1 << 1) & a3) ^ ((a3 << 1) & a1)) & 0xAAAAAAAAu;
    a4 = a2 & a3;
    a4 ^= (a4 & 0xAAAAAAAAu) >> 1;
    a4 ^= (((a2 << 1) & a3) ^ ((a3 << 1) & a2)) & 0xAAAAAAAAu;
    a5 = (x & 0xCCCCCCCCu) >> 2;
    x ^= ((a4 << 2) ^ a4) & 0xCCCCCCCCu;
    a4 = a5 & 0x22222222u;
    a4 |= a4 >> 1;
    a4 ^= (a5 << 1) & 0x22222222u;
    x ^= a4;
    y = ((x & 0xFEFEFEFEu) >> 1) | ((x & 0x01010101u) << 7);
    x &= 0x39393939u;
    x ^= y & 0x3F3F3F3Fu;
    y = ((y & 0xFCFCFCFCu) >> 2) | ((y & 0x03030303u) << 6);
    x ^= y & 0x97979797u;
    y = ((y & 0xFEFEFEFEu) >> 1) | ((y & 0x01010101u) << 7);
    x ^= y & 0x9B9B9B9Bu;
    y = ((y & 0xFEFEFEFEu) >> 1) | ((y & 0x01010101u) << 7);
    x ^= y & 0x3C3C3C3Cu;
    y = ((y & 0xFEFEFEFEu) >> 1) | ((y & 0x01010101u) << 7);
    x ^= y & 0xDDDDDDDDu;
    y = ((y & 0xFEFEFEFEu) >> 1) | ((y & 0x01010101u) << 7);
    x ^= y & 0x72727272u;
    x ^= 0x63636363u;
    *w = x;
}

static void openssl_sub_u64(uint64_t *w)
{
    uint64_t x, y, a1, a2, a3, a4, a5, a6;

    x = *w;
    y = ((x & 0xFEFEFEFEFEFEFEFEuLL) >> 1) | ((x & 0x0101010101010101uLL) << 7);
    x &= 0xDDDDDDDDDDDDDDDDuLL;
    x ^= y & 0x5757575757575757uLL;
    y = ((y & 0xFEFEFEFEFEFEFEFEuLL) >> 1) | ((y & 0x0101010101010101uLL) << 7);
    x ^= y & 0x1C1C1C1C1C1C1C1CuLL;
    y = ((y & 0xFEFEFEFEFEFEFEFEuLL) >> 1) | ((y & 0x0101010101010101uLL) << 7);
    x ^= y & 0x4A4A4A4A4A4A4A4AuLL;
    y = ((y & 0xFEFEFEFEFEFEFEFEuLL) >> 1) | ((y & 0x0101010101010101uLL) << 7);
    x ^= y & 0x4242424242424242uLL;
    y = ((y & 0xFEFEFEFEFEFEFEFEuLL) >> 1) | ((y & 0x0101010101010101uLL) << 7);
    x ^= y & 0x6464646464646464uLL;
    y = ((y & 0xFEFEFEFEFEFEFEFEuLL) >> 1) | ((y & 0x0101010101010101uLL) << 7);
    x ^= y & 0xE0E0E0E0E0E0E0E0uLL;
    a1 = x;
    a1 ^= (x & 0xF0F0F0F0F0F0F0F0uLL) >> 4;
    a2 = ((x & 0xCCCCCCCCCCCCCCCCuLL) >> 2) | ((x & 0x3333333333333333uLL) << 2);
    a3 = x & a1;
    a3 ^= (a3 & 0xAAAAAAAAAAAAAAAAuLL) >> 1;
    a3 ^= (((x << 1) & a1) ^ ((a1 << 1) & x)) & 0xAAAAAAAAAAAAAAAAuLL;
    a4 = a2 & a1;
    a4 ^= (a4 & 0xAAAAAAAAAAAAAAAAuLL) >> 1;
    a4 ^= (((a2 << 1) & a1) ^ ((a1 << 1) & a2)) & 0xAAAAAAAAAAAAAAAAuLL;
    a5 = (a3 & 0xCCCCCCCCCCCCCCCCuLL) >> 2;
    a3 ^= ((a4 << 2) ^ a4) & 0xCCCCCCCCCCCCCCCCuLL;
    a4 = a5 & 0x2222222222222222uLL;
    a4 |= a4 >> 1;
    a4 ^= (a5 << 1) & 0x2222222222222222uLL;
    a3 ^= a4;
    a5 = a3 & 0xA0A0A0A0A0A0A0A0uLL;
    a5 |= a5 >> 1;
    a5 ^= (a3 << 1) & 0xA0A0A0A0A0A0A0A0uLL;
    a4 = a5 & 0xC0C0C0C0C0C0C0C0uLL;
    a6 = a4 >> 2;
    a4 ^= (a5 << 2) & 0xC0C0C0C0C0C0C0C0uLL;
    a5 = a6 & 0x2020202020202020uLL;
    a5 |= a5 >> 1;
    a5 ^= (a6 << 1) & 0x2020202020202020uLL;
    a4 |= a5;
    a3 ^= a4 >> 4;
    a3 &= 0x0F0F0F0F0F0F0F0FuLL;
    a2 = a3;
    a2 ^= (a3 & 0x0C0C0C0C0C0C0C0CuLL) >> 2;
    a4 = a3 & a2;
    a4 ^= (a4 & 0x0A0A0A0A0A0A0A0AuLL) >> 1;
    a4 ^= (((a3 << 1) & a2) ^ ((a2 << 1) & a3)) & 0x0A0A0A0A0A0A0A0AuLL;
    a5 = a4 & 0x0808080808080808uLL;
    a5 |= a5 >> 1;
    a5 ^= (a4 << 1) & 0x0808080808080808uLL;
    a4 ^= a5 >> 2;
    a4 &= 0x0303030303030303uLL;
    a4 ^= (a4 & 0x0202020202020202uLL) >> 1;
    a4 |= a4 << 2;
    a3 = a2 & a4;
    a3 ^= (a3 & 0x0A0A0A0A0A0A0A0AuLL) >> 1;
    a3 ^= (((a2 << 1) & a4) ^ ((a4 << 1) & a2)) & 0x0A0A0A0A0A0A0A0AuLL;
    a3 |= a3 << 4;
    a2 = ((a1 & 0xCCCCCCCCCCCCCCCCuLL) >> 2) | ((a1 & 0x3333333333333333uLL) << 2);
    x = a1 & a3;
    x ^= (x & 0xAAAAAAAAAAAAAAAAuLL) >> 1;
    x ^= (((a1 << 1) & a3) ^ ((a3 << 1) & a1)) & 0xAAAAAAAAAAAAAAAAuLL;
    a4 = a2 & a3;
    a4 ^= (a4 & 0xAAAAAAAAAAAAAAAAuLL) >> 1;
    a4 ^= (((a2 << 1) & a3) ^ ((a3 << 1) & a2)) & 0xAAAAAAAAAAAAAAAAuLL;
    a5 = (x & 0xCCCCCCCCCCCCCCCCuLL) >> 2;
    x ^= ((a4 << 2) ^ a4) & 0xCCCCCCCCCCCCCCCCuLL;
    a4 = a5 & 0x2222222222222222uLL;
    a4 |= a4 >> 1;
    a4 ^= (a5 << 1) & 0x2222222222222222uLL;
    x ^= a4;
    y = ((x & 0xFEFEFEFEFEFEFEFEuLL) >> 1) | ((x & 0x0101010101010101uLL) << 7);
    x &= 0x3939393939393939uLL;
    x ^= y & 0x3F3F3F3F3F3F3F3FuLL;
    y = ((y & 0xFCFCFCFCFCFCFCFCuLL) >> 2) | ((y & 0x0303030303030303uLL) << 6);
    x ^= y & 0x9797979797979797uLL;
    y = ((y & 0xFEFEFEFEFEFEFEFEuLL) >> 1) | ((y & 0x0101010101010101uLL) << 7);
    x ^= y & 0x9B9B9B9B9B9B9B9BuLL;
    y = ((y & 0xFEFEFEFEFEFEFEFEuLL) >> 1) | ((y & 0x0101010101010101uLL) << 7);
    x ^= y & 0x3C3C3C3C3C3C3C3CuLL;
    y = ((y & 0xFEFEFEFEFEFEFEFEuLL) >> 1) | ((y & 0x0101010101010101uLL) << 7);
    x ^= y & 0xDDDDDDDDDDDDDDDDuLL;
    y = ((y & 0xFEFEFEFEFEFEFEFEuLL) >> 1) | ((y & 0x0101010101010101uLL) << 7);
    x ^= y & 0x7272727272727272uLL;
    x ^= 0x6363636363636363uLL;
    *w = x;
}

/*
 * This computes w := (S^-1 * (w + c))^-1
 */
static void openssl_inv_sub_u64(uint64_t *w)
{
    uint64_t x, y, a1, a2, a3, a4, a5, a6;

    x = *w;
    x ^= 0x6363636363636363uLL;
    y = ((x & 0xFEFEFEFEFEFEFEFEuLL) >> 1) | ((x & 0x0101010101010101uLL) << 7);
    x &= 0xFDFDFDFDFDFDFDFDuLL;
    x ^= y & 0x5E5E5E5E5E5E5E5EuLL;
    y = ((y & 0xFEFEFEFEFEFEFEFEuLL) >> 1) | ((y & 0x0101010101010101uLL) << 7);
    x ^= y & 0xF3F3F3F3F3F3F3F3uLL;
    y = ((y & 0xFEFEFEFEFEFEFEFEuLL) >> 1) | ((y & 0x0101010101010101uLL) << 7);
    x ^= y & 0xF5F5F5F5F5F5F5F5uLL;
    y = ((y & 0xFEFEFEFEFEFEFEFEuLL) >> 1) | ((y & 0x0101010101010101uLL) << 7);
    x ^= y & 0x7878787878787878uLL;
    y = ((y & 0xFEFEFEFEFEFEFEFEuLL) >> 1) | ((y & 0x0101010101010101uLL) << 7);
    x ^= y & 0x7777777777777777uLL;
    y = ((y & 0xFEFEFEFEFEFEFEFEuLL) >> 1) | ((y & 0x0101010101010101uLL) << 7);
    x ^= y & 0x1515151515151515uLL;
    y = ((y & 0xFEFEFEFEFEFEFEFEuLL) >> 1) | ((y & 0x0101010101010101uLL) << 7);
    x ^= y & 0xA5A5A5A5A5A5A5A5uLL;
    a1 = x;
    a1 ^= (x & 0xF0F0F0F0F0F0F0F0uLL) >> 4;
    a2 = ((x & 0xCCCCCCCCCCCCCCCCuLL) >> 2) | ((x & 0x3333333333333333uLL) << 2);
    a3 = x & a1;
    a3 ^= (a3 & 0xAAAAAAAAAAAAAAAAuLL) >> 1;
    a3 ^= (((x << 1) & a1) ^ ((a1 << 1) & x)) & 0xAAAAAAAAAAAAAAAAuLL;
    a4 = a2 & a1;
    a4 ^= (a4 & 0xAAAAAAAAAAAAAAAAuLL) >> 1;
    a4 ^= (((a2 << 1) & a1) ^ ((a1 << 1) & a2)) & 0xAAAAAAAAAAAAAAAAuLL;
    a5 = (a3 & 0xCCCCCCCCCCCCCCCCuLL) >> 2;
    a3 ^= ((a4 << 2) ^ a4) & 0xCCCCCCCCCCCCCCCCuLL;
    a4 = a5 & 0x2222222222222222uLL;
    a4 |= a4 >> 1;
    a4 ^= (a5 << 1) & 0x2222222222222222uLL;
    a3 ^= a4;
    a5 = a3 & 0xA0A0A0A0A0A0A0A0uLL;
    a5 |= a5 >> 1;
    a5 ^= (a3 << 1) & 0xA0A0A0A0A0A0A0A0uLL;
    a4 = a5 & 0xC0C0C0C0C0C0C0C0uLL;
    a6 = a4 >> 2;
    a4 ^= (a5 << 2) & 0xC0C0C0C0C0C0C0C0uLL;
    a5 = a6 & 0x2020202020202020uLL;
    a5 |= a5 >> 1;
    a5 ^= (a6 << 1) & 0x2020202020202020uLL;
    a4 |= a5;
    a3 ^= a4 >> 4;
    a3 &= 0x0F0F0F0F0F0F0F0FuLL;
    a2 = a3;
    a2 ^= (a3 & 0x0C0C0C0C0C0C0C0CuLL) >> 2;
    a4 = a3 & a2;
    a4 ^= (a4 & 0x0A0A0A0A0A0A0A0AuLL) >> 1;
    a4 ^= (((a3 << 1) & a2) ^ ((a2 << 1) & a3)) & 0x0A0A0A0A0A0A0A0AuLL;
    a5 = a4 & 0x0808080808080808uLL;
    a5 |= a5 >> 1;
    a5 ^= (a4 << 1) & 0x0808080808080808uLL;
    a4 ^= a5 >> 2;
    a4 &= 0x0303030303030303uLL;
    a4 ^= (a4 & 0x0202020202020202uLL) >> 1;
    a4 |= a4 << 2;
    a3 = a2 & a4;
    a3 ^= (a3 & 0x0A0A0A0A0A0A0A0AuLL) >> 1;
    a3 ^= (((a2 << 1) & a4) ^ ((a4 << 1) & a2)) & 0x0A0A0A0A0A0A0A0AuLL;
    a3 |= a3 << 4;
    a2 = ((a1 & 0xCCCCCCCCCCCCCCCCuLL) >> 2) | ((a1 & 0x3333333333333333uLL) << 2);
    x = a1 & a3;
    x ^= (x & 0xAAAAAAAAAAAAAAAAuLL) >> 1;
    x ^= (((a1 << 1) & a3) ^ ((a3 << 1) & a1)) & 0xAAAAAAAAAAAAAAAAuLL;
    a4 = a2 & a3;
    a4 ^= (a4 & 0xAAAAAAAAAAAAAAAAuLL) >> 1;
    a4 ^= (((a2 << 1) & a3) ^ ((a3 << 1) & a2)) & 0xAAAAAAAAAAAAAAAAuLL;
    a5 = (x & 0xCCCCCCCCCCCCCCCCuLL) >> 2;
    x ^= ((a4 << 2) ^ a4) & 0xCCCCCCCCCCCCCCCCuLL;
    a4 = a5 & 0x2222222222222222uLL;
    a4 |= a4 >> 1;
    a4 ^= (a5 << 1) & 0x2222222222222222uLL;
    x ^= a4;
    y = ((x & 0xFEFEFEFEFEFEFEFEuLL) >> 1) | ((x & 0x0101010101010101uLL) << 7);
    x &= 0xB5B5B5B5B5B5B5B5uLL;
    x ^= y & 0x4040404040404040uLL;
    y = ((y & 0xFEFEFEFEFEFEFEFEuLL) >> 1) | ((y & 0x0101010101010101uLL) << 7);
    x ^= y & 0x8080808080808080uLL;
    y = ((y & 0xFEFEFEFEFEFEFEFEuLL) >> 1) | ((y & 0x0101010101010101uLL) << 7);
    x ^= y & 0x1616161616161616uLL;
    y = ((y & 0xFEFEFEFEFEFEFEFEuLL) >> 1) | ((y & 0x0101010101010101uLL) << 7);
    x ^= y & 0xEBEBEBEBEBEBEBEBuLL;
    y = ((y & 0xFEFEFEFEFEFEFEFEuLL) >> 1) | ((y & 0x0101010101010101uLL) << 7);
    x ^= y & 0x9797979797979797uLL;
    y = ((y & 0xFEFEFEFEFEFEFEFEuLL) >> 1) | ((y & 0x0101010101010101uLL) << 7);
    x ^= y & 0xFBFBFBFBFBFBFBFBuLL;
    y = ((y & 0xFEFEFEFEFEFEFEFEuLL) >> 1) | ((y & 0x0101010101010101uLL) << 7);
    x ^= y & 0x7D7D7D7D7D7D7D7DuLL;
    *w = x;
}

static void openssl_shift_row(uint64_t *state)
{
    unsigned char s[4];
    unsigned char *s0;
    int r;

    s0 = (unsigned char *)state;
    for (r = 0; r < 4; r++) {
        s[0] = s0[0*4 + r];
        s[1] = s0[1*4 + r];
        s[2] = s0[2*4 + r];
        s[3] = s0[3*4 + r];
        s0[0*4 + r] = s[(r+0) % 4];
        s0[1*4 + r] = s[(r+1) % 4];
        s0[2*4 + r] = s[(r+2) % 4];
        s0[3*4 + r] = s[(r+3) % 4];
    }
}

static void openssl_inv_shift_row(uint64_t *state)
{
    unsigned char s[4];
    unsigned char *s0;
    int r;

    s0 = (unsigned char *)state;
    for (r = 0; r < 4; r++) {
        s[0] = s0[0*4 + r];
        s[1] = s0[1*4 + r];
        s[2] = s0[2*4 + r];
        s[3] = s0[3*4 + r];
        s0[0*4 + r] = s[(4-r) % 4];
        s0[1*4 + r] = s[(5-r) % 4];
        s0[2*4 + r] = s[(6-r) % 4];
        s0[3*4 + r] = s[(7-r) % 4];
    }
}

static void openssl_mix_columns(uint64_t *state)
{
    openssl_uni s1;
    openssl_uni s;
    int c;

    for (c = 0; c < 2; c++) {
        s1.d = state[c];
        s.d = s1.d;
        s.d ^= ((s.d & 0xFFFF0000FFFF0000uLL) >> 16)
               | ((s.d & 0x0000FFFF0000FFFFuLL) << 16);
        s.d ^= ((s.d & 0xFF00FF00FF00FF00uLL) >> 8)
               | ((s.d & 0x00FF00FF00FF00FFuLL) << 8);
        s.d ^= s1.d;
        openssl_xtime_u64(&s1.d);
        s.d ^= s1.d;
        s.b[0] ^= s1.b[1];
        s.b[1] ^= s1.b[2];
        s.b[2] ^= s1.b[3];
        s.b[3] ^= s1.b[0];
        s.b[4] ^= s1.b[5];
        s.b[5] ^= s1.b[6];
        s.b[6] ^= s1.b[7];
        s.b[7] ^= s1.b[4];
        state[c] = s.d;
    }
}

static void openssl_inv_mix_columns(uint64_t *state)
{
    openssl_uni s1;
    openssl_uni s;
    int c;

    for (c = 0; c < 2; c++) {
        s1.d = state[c];
        s.d = s1.d;
        s.d ^= ((s.d & 0xFFFF0000FFFF0000uLL) >> 16)
               | ((s.d & 0x0000FFFF0000FFFFuLL) << 16);
        s.d ^= ((s.d & 0xFF00FF00FF00FF00uLL) >> 8)
               | ((s.d & 0x00FF00FF00FF00FFuLL) << 8);
        s.d ^= s1.d;
        openssl_xtime_u64(&s1.d);
        s.d ^= s1.d;
        s.b[0] ^= s1.b[1];
        s.b[1] ^= s1.b[2];
        s.b[2] ^= s1.b[3];
        s.b[3] ^= s1.b[0];
        s.b[4] ^= s1.b[5];
        s.b[5] ^= s1.b[6];
        s.b[6] ^= s1.b[7];
        s.b[7] ^= s1.b[4];
        openssl_xtime_u64(&s1.d);
        s1.d ^= ((s1.d & 0xFFFF0000FFFF0000uLL) >> 16)
                | ((s1.d & 0x0000FFFF0000FFFFuLL) << 16);
        s.d ^= s1.d;
        openssl_xtime_u64(&s1.d);
        s1.d ^= ((s1.d & 0xFF00FF00FF00FF00uLL) >> 8)
                | ((s1.d & 0x00FF00FF00FF00FFuLL) << 8);
        s.d ^= s1.d;
        state[c] = s.d;
    }
}

static void openssl_add_round_key(uint64_t *state, const uint64_t *w)
{
    state[0] ^= w[0];
    state[1] ^= w[1];
}

static void openssl_cipher(const unsigned char *in, unsigned char *out,
                   const uint64_t *w, int nr)
{
    uint64_t state[2];
    int i;

    memcpy(state, in, 16);

    openssl_add_round_key(state, w);

    for (i = 1; i < nr; i++) {
        openssl_sub_u64(&state[0]);
        openssl_sub_u64(&state[1]);
        openssl_shift_row(state);
        openssl_mix_columns(state);
        openssl_add_round_key(state, w + i*2);
    }

    openssl_sub_u64(&state[0]);
    openssl_sub_u64(&state[1]);
    openssl_shift_row(state);
    openssl_add_round_key(state, w + nr*2);

    memcpy(out, state, 16);
}

static void openssl_inv_cipher(const unsigned char *in, unsigned char *out,
                      const uint64_t *w, int nr)

{
    uint64_t state[2];
    int i;

    memcpy(state, in, 16);

    openssl_add_round_key(state, w + nr*2);

    for (i = nr - 1; i > 0; i--) {
        openssl_inv_shift_row(state);
        openssl_inv_sub_u64(&state[0]);
        openssl_inv_sub_u64(&state[1]);
        openssl_add_round_key(state, w + i*2);
        openssl_inv_mix_columns(state);
    }

    openssl_inv_shift_row(state);
    openssl_inv_sub_u64(&state[0]);
    openssl_inv_sub_u64(&state[1]);
    openssl_add_round_key(state, w);

    memcpy(out, state, 16);
}

static void openssl_rot_word(uint32_t *x)
{
    unsigned char *w0;
    unsigned char tmp;

    w0 = (unsigned char *)x;
    tmp = w0[0];
    w0[0] = w0[1];
    w0[1] = w0[2];
    w0[2] = w0[3];
    w0[3] = tmp;
}

static void openssl_key_expansion(const unsigned char *key, uint64_t *w,
                         int nr, int nk)
{
    uint32_t rcon;
    openssl_uni prev;
    uint32_t temp;
    int i, n;

    memcpy(w, key, nk*4);
    memcpy(&rcon, "\1\0\0\0", 4);
    n = nk/2;
    prev.d = w[n-1];
    for (i = n; i < (nr+1)*2; i++) {
        temp = prev.w[1];
        if (i % n == 0) {
            openssl_rot_word(&temp);
            openssl_sub_u32(&temp);
            temp ^= rcon;
            openssl_xtime_u32(&rcon);
        } else if (nk > 6 && i % n == 2) {
            openssl_sub_u32(&temp);
        }
        prev.d = w[i-n];
        prev.w[0] ^= temp;
        prev.w[1] ^= prev.w[0];
        w[i] = prev.d;
    }
}

int muggle_openssl_aes_set_key(
	const unsigned char *key,
	const int bits,
	struct muggle_aes_sub_keys *sk)
{
    uint64_t *rk;

    rk = (uint64_t*)sk->rd_key;

    if (bits == 128)
        sk->rounds = 10;
    else if (bits == 192)
        sk->rounds = 12;
    else if (bits == 256)
        sk->rounds = 14;
	else
	{
		MUGGLE_ASSERT_MSG(
			bits == 128 || bits == 192 || bits == 256,
			"AES key setup only support bit size: 128/192/256");
		return MUGGLE_ERR_CRYPT_KEY_SIZE;
	}

    openssl_key_expansion(key, rk, sk->rounds, bits/32);
    return 0;
}

void muggle_openssl_aes_encrypt(
	const unsigned char *in,
	unsigned char *out,
    const struct muggle_aes_sub_keys *sk)
{
    const uint64_t *rk;
    rk = (uint64_t*)sk->rd_key;
    openssl_cipher(in, out, rk, sk->rounds);
}

void muggle_openssl_aes_decrypt(
	const unsigned char *in,
	unsigned char *out,
    const struct muggle_aes_sub_keys *sk)
{
    const uint64_t *rk;
    rk = (uint64_t*)sk->rd_key;
    openssl_inv_cipher(in, out, rk, sk->rounds);
}
