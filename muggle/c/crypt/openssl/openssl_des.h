/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

/*
 * Compatibility with openssl
 *
 * this header file and the corresponding source file are
 * all copied and modified from openssl(Apache License v2.0)
 *
 * */

#ifndef MUGGLE_C_OPENSSL_DES_H_
#define MUGGLE_C_OPENSSL_DES_H_

#include "muggle/c/base/macro.h"
#include "muggle/c/crypt/crypt_utils.h"

EXTERN_C_BEGIN

/*

    permutation operation in openssl is very interesting, as comment in openssl 
    said: "The problem is more of a geometric problem that random bit fiddling"

    a simple case is 16 bit writen as 4x4 matrix:
    
    PERM_OP:
        T = [(L >> n) ^ R] & m
        R = R ^ T
        l = L ^ (T << n)
    
    L = l00 l01 l02 l03  R = r00 r01 r02 r03  M = m00 m01 m02 m03
        l04 l05 l06 l07      r04 r05 r06 r07      m04 m05 m06 m07
        l08 l09 l10 l11      r08 r09 r10 r11      m08 m09 m10 m11
        l12 l13 l14 l15      r12 r13 r14 r15      m12 m13 m14 m15
    
    e.g. shift 1 with mask 0x55(01010101)
    T = (l01^r00) 0 (l03^r02) 0
	    (l05^r04) 0 (l07^r06) 0
	    (l09^r08) 0 (l11^r10) 0     
	    (l13^r12) 0 (l15^r14) 0     

    R = R ^ T = R = l01 r01 l03 r03 
                    l05 r05 l07 r07 
                    l09 r09 l11 r11 
                    l13 r13 l15 r15 
	
	L = L ^ (T << 1) = l00 r00 l02 r02
                       l04 r04 l06 r06
                       l08 r08 l10 r10
                       l12 r12 l14 r14
	
    easy to roll out other operations: 
        shift 2 with mask 0x33... (00110011)
        shift 4 with mask 0x0f... (00001111)
        shift 8 with mask 0x00ff... (0000 0000 1111 1111)
		shift 16 with mask 0x0000ffff... (0000 0000 0000 0000 1111 1111 1111 1111)

*/

#define MUGGLE_OPENSSL_DES_PERM_OP(a,b,t,n,m) \
	((t)=((((a)>>(n))^(b))&(m)), \
    (b)^=(t),\
    (a)^=((t)<<(n)))

#define MUGGLE_OPENSSL_DES_HPERM_OP(a,t,n,m) \
	((t)=((((a)<<(16-(n)))^(a))&(m)), \
	(a)=(a)^(t)^(t>>(16-(n))))\

#define MUGGLE_OPENSSL_DES_ROTATE(v, n) \
	((v)>>(n))+((v)<<(32-(n)))

#define MUGGLE_OPENSSL_DES_IP(l,r) \
	{ \
		register uint32_t tt; \
		MUGGLE_OPENSSL_DES_PERM_OP(r,l,tt, 4,0x0f0f0f0fL); \
		MUGGLE_OPENSSL_DES_PERM_OP(l,r,tt,16,0x0000ffffL); \
		MUGGLE_OPENSSL_DES_PERM_OP(r,l,tt, 2,0x33333333L); \
		MUGGLE_OPENSSL_DES_PERM_OP(l,r,tt, 8,0x00ff00ffL); \
		MUGGLE_OPENSSL_DES_PERM_OP(r,l,tt, 1,0x55555555L); \
		l=MUGGLE_OPENSSL_DES_ROTATE(l,29) & 0xffffffffL; \
		r=MUGGLE_OPENSSL_DES_ROTATE(r,29) & 0xffffffffL; \
	}

#define MUGGLE_OPENSSL_DES_FP(l,r) \
	{ \
 		l=MUGGLE_OPENSSL_DES_ROTATE(l,3) & 0xffffffffL; \
 		r=MUGGLE_OPENSSL_DES_ROTATE(r,3) & 0xffffffffL; \
		register uint32_t tt; \
		MUGGLE_OPENSSL_DES_PERM_OP(l,r,tt, 1,0x55555555L); \
		MUGGLE_OPENSSL_DES_PERM_OP(r,l,tt, 8,0x00ff00ffL); \
		MUGGLE_OPENSSL_DES_PERM_OP(l,r,tt, 2,0x33333333L); \
		MUGGLE_OPENSSL_DES_PERM_OP(r,l,tt,16,0x0000ffffL); \
		MUGGLE_OPENSSL_DES_PERM_OP(l,r,tt, 4,0x0f0f0f0fL); \
	}

struct muggle_des_subkeys;

/*
 * equal to openssl - DES_set_key_checked
 * @key: 64bit char array
 * RETURN:
 * 	return 0 if key parity is odd (correct)
 * 	return -1 if key parity error
 * 	return -2 if illegal weak key
 * */
MUGGLE_CC_EXPORT
int muggle_openssl_des_set_key_checked(unsigned char *key, struct muggle_des_subkeys *ks);

/*
 * equal to openssl - DES_set_key_unchecked
 * @key: 64bit char array
 * */
MUGGLE_CC_EXPORT
void muggle_openssl_des_set_key_unchecked(unsigned char *key, struct muggle_des_subkeys *ks);

/*
 * equal to openssl - D_ENCRYPT after first sentence
 * @block: (extend r) xor subkey
 * */
MUGGLE_CC_EXPORT
uint32_t muggle_openssl_des_sbox(muggle_64bit_block_t block);

EXTERN_C_END

#endif
