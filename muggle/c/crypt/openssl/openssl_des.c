/******************************************************************************
 *  @file         openssl_des.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-16
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        
 *  mugglec crypt compatible with openssl DES
 *
 *  this file are all copied and modified from openssl(Apache License v2.0)
 *****************************************************************************/
 
#include "openssl_des.h"
#include <stdio.h>
#include <string.h>
#include "muggle/c/crypt/parity.h"
#include "muggle/c/crypt/des.h"
#include "muggle/c/crypt/internal/internal_des.h"
#include "muggle/c/crypt/crypt_utils.h"

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
	}

#define MUGGLE_OPENSSL_DES_FP(l,r) \
	{ \
		register uint32_t tt; \
		MUGGLE_OPENSSL_DES_PERM_OP(l,r,tt, 1,0x55555555L); \
		MUGGLE_OPENSSL_DES_PERM_OP(r,l,tt, 8,0x00ff00ffL); \
		MUGGLE_OPENSSL_DES_PERM_OP(l,r,tt, 2,0x33333333L); \
		MUGGLE_OPENSSL_DES_PERM_OP(r,l,tt,16,0x0000ffffL); \
		MUGGLE_OPENSSL_DES_PERM_OP(l,r,tt, 4,0x0f0f0f0fL); \
	}

#define MUGGLE_OPENSSL_D_ENCRYPT(l, r, ks_u32, idx) {\
	uint32_t u = (r)^(ks_u32)[idx]; \
	uint32_t t = r^(ks_u32)[idx+1]; \
	t = MUGGLE_OPENSSL_DES_ROTATE(t, 4); \
	(l) ^= \
		openssl_des_sptrans[0][(u>> 2L)&0x3f]^ \
		openssl_des_sptrans[2][(u>>10L)&0x3f]^ \
		openssl_des_sptrans[4][(u>>18L)&0x3f]^ \
		openssl_des_sptrans[6][(u>>26L)&0x3f]^ \
		openssl_des_sptrans[1][(t>> 2L)&0x3f]^ \
		openssl_des_sptrans[3][(t>>10L)&0x3f]^ \
		openssl_des_sptrans[5][(t>>18L)&0x3f]^ \
		openssl_des_sptrans[7][(t>>26L)&0x3f]; \
}

#define MUGGLE_OPENSSL_C2L(c, l) \
	l = \
		(((uint32_t)(*((c)+0)) << 0L)& 0x000000ffL) | \
		(((uint32_t)(*((c)+1)) << 8L) & 0x0000ff00L) | \
		(((uint32_t)(*((c)+2)) << 16L) & 0x00ff0000L) | \
		(((uint32_t)(*((c)+3)) << 24L) & 0xff000000L)

#define MUGGLE_OPENSSL_L2C(l, c) (\
		*((c)+0)=(unsigned char)(((l)>>24L)&0xff), \
		*((c)+1)=(unsigned char)(((l)>>16L)&0xff), \
		*((c)+2)=(unsigned char)(((l)>>8L)&0xff), \
		*((c)+3)=(unsigned char)(((l)>>0L)&0xff))

static const uint32_t openssl_des_skb[8][64] = {
    {
     /* for C bits (numbered as per FIPS 46) 1 2 3 4 5 6 */
     0x00000000L, 0x00000010L, 0x20000000L, 0x20000010L,
     0x00010000L, 0x00010010L, 0x20010000L, 0x20010010L,
     0x00000800L, 0x00000810L, 0x20000800L, 0x20000810L,
     0x00010800L, 0x00010810L, 0x20010800L, 0x20010810L,
     0x00000020L, 0x00000030L, 0x20000020L, 0x20000030L,
     0x00010020L, 0x00010030L, 0x20010020L, 0x20010030L,
     0x00000820L, 0x00000830L, 0x20000820L, 0x20000830L,
     0x00010820L, 0x00010830L, 0x20010820L, 0x20010830L,
     0x00080000L, 0x00080010L, 0x20080000L, 0x20080010L,
     0x00090000L, 0x00090010L, 0x20090000L, 0x20090010L,
     0x00080800L, 0x00080810L, 0x20080800L, 0x20080810L,
     0x00090800L, 0x00090810L, 0x20090800L, 0x20090810L,
     0x00080020L, 0x00080030L, 0x20080020L, 0x20080030L,
     0x00090020L, 0x00090030L, 0x20090020L, 0x20090030L,
     0x00080820L, 0x00080830L, 0x20080820L, 0x20080830L,
     0x00090820L, 0x00090830L, 0x20090820L, 0x20090830L,
     },
    {
     /* for C bits (numbered as per FIPS 46) 7 8 10 11 12 13 */
     0x00000000L, 0x02000000L, 0x00002000L, 0x02002000L,
     0x00200000L, 0x02200000L, 0x00202000L, 0x02202000L,
     0x00000004L, 0x02000004L, 0x00002004L, 0x02002004L,
     0x00200004L, 0x02200004L, 0x00202004L, 0x02202004L,
     0x00000400L, 0x02000400L, 0x00002400L, 0x02002400L,
     0x00200400L, 0x02200400L, 0x00202400L, 0x02202400L,
     0x00000404L, 0x02000404L, 0x00002404L, 0x02002404L,
     0x00200404L, 0x02200404L, 0x00202404L, 0x02202404L,
     0x10000000L, 0x12000000L, 0x10002000L, 0x12002000L,
     0x10200000L, 0x12200000L, 0x10202000L, 0x12202000L,
     0x10000004L, 0x12000004L, 0x10002004L, 0x12002004L,
     0x10200004L, 0x12200004L, 0x10202004L, 0x12202004L,
     0x10000400L, 0x12000400L, 0x10002400L, 0x12002400L,
     0x10200400L, 0x12200400L, 0x10202400L, 0x12202400L,
     0x10000404L, 0x12000404L, 0x10002404L, 0x12002404L,
     0x10200404L, 0x12200404L, 0x10202404L, 0x12202404L,
     },
    {
     /* for C bits (numbered as per FIPS 46) 14 15 16 17 19 20 */
     0x00000000L, 0x00000001L, 0x00040000L, 0x00040001L,
     0x01000000L, 0x01000001L, 0x01040000L, 0x01040001L,
     0x00000002L, 0x00000003L, 0x00040002L, 0x00040003L,
     0x01000002L, 0x01000003L, 0x01040002L, 0x01040003L,
     0x00000200L, 0x00000201L, 0x00040200L, 0x00040201L,
     0x01000200L, 0x01000201L, 0x01040200L, 0x01040201L,
     0x00000202L, 0x00000203L, 0x00040202L, 0x00040203L,
     0x01000202L, 0x01000203L, 0x01040202L, 0x01040203L,
     0x08000000L, 0x08000001L, 0x08040000L, 0x08040001L,
     0x09000000L, 0x09000001L, 0x09040000L, 0x09040001L,
     0x08000002L, 0x08000003L, 0x08040002L, 0x08040003L,
     0x09000002L, 0x09000003L, 0x09040002L, 0x09040003L,
     0x08000200L, 0x08000201L, 0x08040200L, 0x08040201L,
     0x09000200L, 0x09000201L, 0x09040200L, 0x09040201L,
     0x08000202L, 0x08000203L, 0x08040202L, 0x08040203L,
     0x09000202L, 0x09000203L, 0x09040202L, 0x09040203L,
     },
    {
     /* for C bits (numbered as per FIPS 46) 21 23 24 26 27 28 */
     0x00000000L, 0x00100000L, 0x00000100L, 0x00100100L,
     0x00000008L, 0x00100008L, 0x00000108L, 0x00100108L,
     0x00001000L, 0x00101000L, 0x00001100L, 0x00101100L,
     0x00001008L, 0x00101008L, 0x00001108L, 0x00101108L,
     0x04000000L, 0x04100000L, 0x04000100L, 0x04100100L,
     0x04000008L, 0x04100008L, 0x04000108L, 0x04100108L,
     0x04001000L, 0x04101000L, 0x04001100L, 0x04101100L,
     0x04001008L, 0x04101008L, 0x04001108L, 0x04101108L,
     0x00020000L, 0x00120000L, 0x00020100L, 0x00120100L,
     0x00020008L, 0x00120008L, 0x00020108L, 0x00120108L,
     0x00021000L, 0x00121000L, 0x00021100L, 0x00121100L,
     0x00021008L, 0x00121008L, 0x00021108L, 0x00121108L,
     0x04020000L, 0x04120000L, 0x04020100L, 0x04120100L,
     0x04020008L, 0x04120008L, 0x04020108L, 0x04120108L,
     0x04021000L, 0x04121000L, 0x04021100L, 0x04121100L,
     0x04021008L, 0x04121008L, 0x04021108L, 0x04121108L,
     },
    {
     /* for D bits (numbered as per FIPS 46) 1 2 3 4 5 6 */
     0x00000000L, 0x10000000L, 0x00010000L, 0x10010000L,
     0x00000004L, 0x10000004L, 0x00010004L, 0x10010004L,
     0x20000000L, 0x30000000L, 0x20010000L, 0x30010000L,
     0x20000004L, 0x30000004L, 0x20010004L, 0x30010004L,
     0x00100000L, 0x10100000L, 0x00110000L, 0x10110000L,
     0x00100004L, 0x10100004L, 0x00110004L, 0x10110004L,
     0x20100000L, 0x30100000L, 0x20110000L, 0x30110000L,
     0x20100004L, 0x30100004L, 0x20110004L, 0x30110004L,
     0x00001000L, 0x10001000L, 0x00011000L, 0x10011000L,
     0x00001004L, 0x10001004L, 0x00011004L, 0x10011004L,
     0x20001000L, 0x30001000L, 0x20011000L, 0x30011000L,
     0x20001004L, 0x30001004L, 0x20011004L, 0x30011004L,
     0x00101000L, 0x10101000L, 0x00111000L, 0x10111000L,
     0x00101004L, 0x10101004L, 0x00111004L, 0x10111004L,
     0x20101000L, 0x30101000L, 0x20111000L, 0x30111000L,
     0x20101004L, 0x30101004L, 0x20111004L, 0x30111004L,
     },
    {
     /* for D bits (numbered as per FIPS 46) 8 9 11 12 13 14 */
     0x00000000L, 0x08000000L, 0x00000008L, 0x08000008L,
     0x00000400L, 0x08000400L, 0x00000408L, 0x08000408L,
     0x00020000L, 0x08020000L, 0x00020008L, 0x08020008L,
     0x00020400L, 0x08020400L, 0x00020408L, 0x08020408L,
     0x00000001L, 0x08000001L, 0x00000009L, 0x08000009L,
     0x00000401L, 0x08000401L, 0x00000409L, 0x08000409L,
     0x00020001L, 0x08020001L, 0x00020009L, 0x08020009L,
     0x00020401L, 0x08020401L, 0x00020409L, 0x08020409L,
     0x02000000L, 0x0A000000L, 0x02000008L, 0x0A000008L,
     0x02000400L, 0x0A000400L, 0x02000408L, 0x0A000408L,
     0x02020000L, 0x0A020000L, 0x02020008L, 0x0A020008L,
     0x02020400L, 0x0A020400L, 0x02020408L, 0x0A020408L,
     0x02000001L, 0x0A000001L, 0x02000009L, 0x0A000009L,
     0x02000401L, 0x0A000401L, 0x02000409L, 0x0A000409L,
     0x02020001L, 0x0A020001L, 0x02020009L, 0x0A020009L,
     0x02020401L, 0x0A020401L, 0x02020409L, 0x0A020409L,
     },
    {
     /* for D bits (numbered as per FIPS 46) 16 17 18 19 20 21 */
     0x00000000L, 0x00000100L, 0x00080000L, 0x00080100L,
     0x01000000L, 0x01000100L, 0x01080000L, 0x01080100L,
     0x00000010L, 0x00000110L, 0x00080010L, 0x00080110L,
     0x01000010L, 0x01000110L, 0x01080010L, 0x01080110L,
     0x00200000L, 0x00200100L, 0x00280000L, 0x00280100L,
     0x01200000L, 0x01200100L, 0x01280000L, 0x01280100L,
     0x00200010L, 0x00200110L, 0x00280010L, 0x00280110L,
     0x01200010L, 0x01200110L, 0x01280010L, 0x01280110L,
     0x00000200L, 0x00000300L, 0x00080200L, 0x00080300L,
     0x01000200L, 0x01000300L, 0x01080200L, 0x01080300L,
     0x00000210L, 0x00000310L, 0x00080210L, 0x00080310L,
     0x01000210L, 0x01000310L, 0x01080210L, 0x01080310L,
     0x00200200L, 0x00200300L, 0x00280200L, 0x00280300L,
     0x01200200L, 0x01200300L, 0x01280200L, 0x01280300L,
     0x00200210L, 0x00200310L, 0x00280210L, 0x00280310L,
     0x01200210L, 0x01200310L, 0x01280210L, 0x01280310L,
     },
    {
     /* for D bits (numbered as per FIPS 46) 22 23 24 25 27 28 */
     0x00000000L, 0x04000000L, 0x00040000L, 0x04040000L,
     0x00000002L, 0x04000002L, 0x00040002L, 0x04040002L,
     0x00002000L, 0x04002000L, 0x00042000L, 0x04042000L,
     0x00002002L, 0x04002002L, 0x00042002L, 0x04042002L,
     0x00000020L, 0x04000020L, 0x00040020L, 0x04040020L,
     0x00000022L, 0x04000022L, 0x00040022L, 0x04040022L,
     0x00002020L, 0x04002020L, 0x00042020L, 0x04042020L,
     0x00002022L, 0x04002022L, 0x00042022L, 0x04042022L,
     0x00000800L, 0x04000800L, 0x00040800L, 0x04040800L,
     0x00000802L, 0x04000802L, 0x00040802L, 0x04040802L,
     0x00002800L, 0x04002800L, 0x00042800L, 0x04042800L,
     0x00002802L, 0x04002802L, 0x00042802L, 0x04042802L,
     0x00000820L, 0x04000820L, 0x00040820L, 0x04040820L,
     0x00000822L, 0x04000822L, 0x00040822L, 0x04040822L,
     0x00002820L, 0x04002820L, 0x00042820L, 0x04042820L,
     0x00002822L, 0x04002822L, 0x00042822L, 0x04042822L,
     }
};

#define NUM_WEAK_KEY    16
typedef unsigned char openssl_des_key_block[8];
static const openssl_des_key_block openssl_weak_keys[NUM_WEAK_KEY] = {
    /* weak keys */
    {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01},
    {0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE},
    {0x1F, 0x1F, 0x1F, 0x1F, 0x0E, 0x0E, 0x0E, 0x0E},
    {0xE0, 0xE0, 0xE0, 0xE0, 0xF1, 0xF1, 0xF1, 0xF1},
    /* semi-weak keys */
    {0x01, 0xFE, 0x01, 0xFE, 0x01, 0xFE, 0x01, 0xFE},
    {0xFE, 0x01, 0xFE, 0x01, 0xFE, 0x01, 0xFE, 0x01},
    {0x1F, 0xE0, 0x1F, 0xE0, 0x0E, 0xF1, 0x0E, 0xF1},
    {0xE0, 0x1F, 0xE0, 0x1F, 0xF1, 0x0E, 0xF1, 0x0E},
    {0x01, 0xE0, 0x01, 0xE0, 0x01, 0xF1, 0x01, 0xF1},
    {0xE0, 0x01, 0xE0, 0x01, 0xF1, 0x01, 0xF1, 0x01},
    {0x1F, 0xFE, 0x1F, 0xFE, 0x0E, 0xFE, 0x0E, 0xFE},
    {0xFE, 0x1F, 0xFE, 0x1F, 0xFE, 0x0E, 0xFE, 0x0E},
    {0x01, 0x1F, 0x01, 0x1F, 0x01, 0x0E, 0x01, 0x0E},
    {0x1F, 0x01, 0x1F, 0x01, 0x0E, 0x01, 0x0E, 0x01},
    {0xE0, 0xFE, 0xE0, 0xFE, 0xF1, 0xFE, 0xF1, 0xFE},
    {0xFE, 0xE0, 0xFE, 0xE0, 0xFE, 0xF1, 0xFE, 0xF1}
};

const uint32_t openssl_des_sptrans[8][64] = {
    {
        /* nibble 0 */
        0x02080800L, 0x00080000L, 0x02000002L, 0x02080802L,
        0x02000000L, 0x00080802L, 0x00080002L, 0x02000002L,
        0x00080802L, 0x02080800L, 0x02080000L, 0x00000802L,
        0x02000802L, 0x02000000L, 0x00000000L, 0x00080002L,
        0x00080000L, 0x00000002L, 0x02000800L, 0x00080800L,
        0x02080802L, 0x02080000L, 0x00000802L, 0x02000800L,
        0x00000002L, 0x00000800L, 0x00080800L, 0x02080002L,
        0x00000800L, 0x02000802L, 0x02080002L, 0x00000000L,
        0x00000000L, 0x02080802L, 0x02000800L, 0x00080002L,
        0x02080800L, 0x00080000L, 0x00000802L, 0x02000800L,
        0x02080002L, 0x00000800L, 0x00080800L, 0x02000002L,
        0x00080802L, 0x00000002L, 0x02000002L, 0x02080000L,
        0x02080802L, 0x00080800L, 0x02080000L, 0x02000802L,
        0x02000000L, 0x00000802L, 0x00080002L, 0x00000000L,
        0x00080000L, 0x02000000L, 0x02000802L, 0x02080800L,
        0x00000002L, 0x02080002L, 0x00000800L, 0x00080802L,
    },
    {
        /* nibble 1 */
        0x40108010L, 0x00000000L, 0x00108000L, 0x40100000L,
        0x40000010L, 0x00008010L, 0x40008000L, 0x00108000L,
        0x00008000L, 0x40100010L, 0x00000010L, 0x40008000L,
        0x00100010L, 0x40108000L, 0x40100000L, 0x00000010L,
        0x00100000L, 0x40008010L, 0x40100010L, 0x00008000L,
        0x00108010L, 0x40000000L, 0x00000000L, 0x00100010L,
        0x40008010L, 0x00108010L, 0x40108000L, 0x40000010L,
        0x40000000L, 0x00100000L, 0x00008010L, 0x40108010L,
        0x00100010L, 0x40108000L, 0x40008000L, 0x00108010L,
        0x40108010L, 0x00100010L, 0x40000010L, 0x00000000L,
        0x40000000L, 0x00008010L, 0x00100000L, 0x40100010L,
        0x00008000L, 0x40000000L, 0x00108010L, 0x40008010L,
        0x40108000L, 0x00008000L, 0x00000000L, 0x40000010L,
        0x00000010L, 0x40108010L, 0x00108000L, 0x40100000L,
        0x40100010L, 0x00100000L, 0x00008010L, 0x40008000L,
        0x40008010L, 0x00000010L, 0x40100000L, 0x00108000L,
    },
    {
        /* nibble 2 */
        0x04000001L, 0x04040100L, 0x00000100L, 0x04000101L,
        0x00040001L, 0x04000000L, 0x04000101L, 0x00040100L,
        0x04000100L, 0x00040000L, 0x04040000L, 0x00000001L,
        0x04040101L, 0x00000101L, 0x00000001L, 0x04040001L,
        0x00000000L, 0x00040001L, 0x04040100L, 0x00000100L,
        0x00000101L, 0x04040101L, 0x00040000L, 0x04000001L,
        0x04040001L, 0x04000100L, 0x00040101L, 0x04040000L,
        0x00040100L, 0x00000000L, 0x04000000L, 0x00040101L,
        0x04040100L, 0x00000100L, 0x00000001L, 0x00040000L,
        0x00000101L, 0x00040001L, 0x04040000L, 0x04000101L,
        0x00000000L, 0x04040100L, 0x00040100L, 0x04040001L,
        0x00040001L, 0x04000000L, 0x04040101L, 0x00000001L,
        0x00040101L, 0x04000001L, 0x04000000L, 0x04040101L,
        0x00040000L, 0x04000100L, 0x04000101L, 0x00040100L,
        0x04000100L, 0x00000000L, 0x04040001L, 0x00000101L,
        0x04000001L, 0x00040101L, 0x00000100L, 0x04040000L,
    },
    {
        /* nibble 3 */
        0x00401008L, 0x10001000L, 0x00000008L, 0x10401008L,
        0x00000000L, 0x10400000L, 0x10001008L, 0x00400008L,
        0x10401000L, 0x10000008L, 0x10000000L, 0x00001008L,
        0x10000008L, 0x00401008L, 0x00400000L, 0x10000000L,
        0x10400008L, 0x00401000L, 0x00001000L, 0x00000008L,
        0x00401000L, 0x10001008L, 0x10400000L, 0x00001000L,
        0x00001008L, 0x00000000L, 0x00400008L, 0x10401000L,
        0x10001000L, 0x10400008L, 0x10401008L, 0x00400000L,
        0x10400008L, 0x00001008L, 0x00400000L, 0x10000008L,
        0x00401000L, 0x10001000L, 0x00000008L, 0x10400000L,
        0x10001008L, 0x00000000L, 0x00001000L, 0x00400008L,
        0x00000000L, 0x10400008L, 0x10401000L, 0x00001000L,
        0x10000000L, 0x10401008L, 0x00401008L, 0x00400000L,
        0x10401008L, 0x00000008L, 0x10001000L, 0x00401008L,
        0x00400008L, 0x00401000L, 0x10400000L, 0x10001008L,
        0x00001008L, 0x10000000L, 0x10000008L, 0x10401000L,
    },
    {
        /* nibble 4 */
        0x08000000L, 0x00010000L, 0x00000400L, 0x08010420L,
        0x08010020L, 0x08000400L, 0x00010420L, 0x08010000L,
        0x00010000L, 0x00000020L, 0x08000020L, 0x00010400L,
        0x08000420L, 0x08010020L, 0x08010400L, 0x00000000L,
        0x00010400L, 0x08000000L, 0x00010020L, 0x00000420L,
        0x08000400L, 0x00010420L, 0x00000000L, 0x08000020L,
        0x00000020L, 0x08000420L, 0x08010420L, 0x00010020L,
        0x08010000L, 0x00000400L, 0x00000420L, 0x08010400L,
        0x08010400L, 0x08000420L, 0x00010020L, 0x08010000L,
        0x00010000L, 0x00000020L, 0x08000020L, 0x08000400L,
        0x08000000L, 0x00010400L, 0x08010420L, 0x00000000L,
        0x00010420L, 0x08000000L, 0x00000400L, 0x00010020L,
        0x08000420L, 0x00000400L, 0x00000000L, 0x08010420L,
        0x08010020L, 0x08010400L, 0x00000420L, 0x00010000L,
        0x00010400L, 0x08010020L, 0x08000400L, 0x00000420L,
        0x00000020L, 0x00010420L, 0x08010000L, 0x08000020L,
    },
    {
        /* nibble 5 */
        0x80000040L, 0x00200040L, 0x00000000L, 0x80202000L,
        0x00200040L, 0x00002000L, 0x80002040L, 0x00200000L,
        0x00002040L, 0x80202040L, 0x00202000L, 0x80000000L,
        0x80002000L, 0x80000040L, 0x80200000L, 0x00202040L,
        0x00200000L, 0x80002040L, 0x80200040L, 0x00000000L,
        0x00002000L, 0x00000040L, 0x80202000L, 0x80200040L,
        0x80202040L, 0x80200000L, 0x80000000L, 0x00002040L,
        0x00000040L, 0x00202000L, 0x00202040L, 0x80002000L,
        0x00002040L, 0x80000000L, 0x80002000L, 0x00202040L,
        0x80202000L, 0x00200040L, 0x00000000L, 0x80002000L,
        0x80000000L, 0x00002000L, 0x80200040L, 0x00200000L,
        0x00200040L, 0x80202040L, 0x00202000L, 0x00000040L,
        0x80202040L, 0x00202000L, 0x00200000L, 0x80002040L,
        0x80000040L, 0x80200000L, 0x00202040L, 0x00000000L,
        0x00002000L, 0x80000040L, 0x80002040L, 0x80202000L,
        0x80200000L, 0x00002040L, 0x00000040L, 0x80200040L,
    },
    {
        /* nibble 6 */
        0x00004000L, 0x00000200L, 0x01000200L, 0x01000004L,
        0x01004204L, 0x00004004L, 0x00004200L, 0x00000000L,
        0x01000000L, 0x01000204L, 0x00000204L, 0x01004000L,
        0x00000004L, 0x01004200L, 0x01004000L, 0x00000204L,
        0x01000204L, 0x00004000L, 0x00004004L, 0x01004204L,
        0x00000000L, 0x01000200L, 0x01000004L, 0x00004200L,
        0x01004004L, 0x00004204L, 0x01004200L, 0x00000004L,
        0x00004204L, 0x01004004L, 0x00000200L, 0x01000000L,
        0x00004204L, 0x01004000L, 0x01004004L, 0x00000204L,
        0x00004000L, 0x00000200L, 0x01000000L, 0x01004004L,
        0x01000204L, 0x00004204L, 0x00004200L, 0x00000000L,
        0x00000200L, 0x01000004L, 0x00000004L, 0x01000200L,
        0x00000000L, 0x01000204L, 0x01000200L, 0x00004200L,
        0x00000204L, 0x00004000L, 0x01004204L, 0x01000000L,
        0x01004200L, 0x00000004L, 0x00004004L, 0x01004204L,
        0x01000004L, 0x01004200L, 0x01004000L, 0x00004004L,
    },
    {
        /* nibble 7 */
        0x20800080L, 0x20820000L, 0x00020080L, 0x00000000L,
        0x20020000L, 0x00800080L, 0x20800000L, 0x20820080L,
        0x00000080L, 0x20000000L, 0x00820000L, 0x00020080L,
        0x00820080L, 0x20020080L, 0x20000080L, 0x20800000L,
        0x00020000L, 0x00820080L, 0x00800080L, 0x20020000L,
        0x20820080L, 0x20000080L, 0x00000000L, 0x00820000L,
        0x20000000L, 0x00800000L, 0x20020080L, 0x20800080L,
        0x00800000L, 0x00020000L, 0x20820000L, 0x00000080L,
        0x00800000L, 0x00020000L, 0x20000080L, 0x20820080L,
        0x00020080L, 0x20000000L, 0x00000000L, 0x00820000L,
        0x20800080L, 0x20020080L, 0x20020000L, 0x00800080L,
        0x20820000L, 0x00000080L, 0x00800080L, 0x20020000L,
        0x20820080L, 0x00800000L, 0x20800000L, 0x20000080L,
        0x00820000L, 0x00020080L, 0x20020080L, 0x20800000L,
        0x00000080L, 0x20820000L, 0x00820080L, 0x00000000L,
        0x20000000L, 0x20800080L, 0x00020000L, 0x00820080L,
    }
};

static int openssl_des_check_key_parity(unsigned char *key)
{
	for (int i = 0; i < 8; ++i)
	{
		if (!muggle_parity_check_odd(key[i]))
		{
			return 0;
		}
	}

	return 1;
}

static int openssl_des_is_weak_key(unsigned char *key)
{
	for (int i = 0; i < NUM_WEAK_KEY; ++i)
	{
		if (memcmp(openssl_weak_keys[i], key, sizeof(openssl_des_key_block)) == 0)
		{
			return 1;
		}
	}
	return 0;
}

/*
 * equal to openssl - DES_set_key_unchecked
 * @key: 64bit char array
 * */
void muggle_openssl_des_set_key_unchecked(unsigned char *key, struct muggle_des_subkeys *ks)
{
	uint32_t c, d, t;

	// openssl des c2l
	MUGGLE_OPENSSL_C2L(key+0,c);
	MUGGLE_OPENSSL_C2L(key+4,d);

	// PC-1
	MUGGLE_OPENSSL_DES_PERM_OP(d, c, t, 4, 0x0f0f0f0fL);
    MUGGLE_OPENSSL_DES_HPERM_OP(c, t, -2, 0xcccc0000L);
    MUGGLE_OPENSSL_DES_HPERM_OP(d, t, -2, 0xcccc0000L);
    MUGGLE_OPENSSL_DES_PERM_OP(d, c, t, 1, 0x55555555L);
    MUGGLE_OPENSSL_DES_PERM_OP(c, d, t, 8, 0x00ff00ffL);
    MUGGLE_OPENSSL_DES_PERM_OP(d, c, t, 1, 0x55555555L);
    d = (((d & 0x000000ffL) << 16L) | (d & 0x0000ff00L) |
         ((d & 0x00ff0000L) >> 16L) | ((c & 0xf0000000L) >> 4L));
    c &= 0x0fffffffL;

	// 16 rounds shift
	uint32_t s, t2;
	const int64_t shifts1[16] = {
		 1L, 1L, 2L, 2L,
		 2L, 2L, 2L, 2L,
		 1L, 2L, 2L, 2L,
		 2L, 2L, 2L, 1L
	};
	const int64_t shifts2[16] = {
		 27L, 27L, 26L, 26L,
		 26L, 26L, 26L, 26L,
		 27L, 26L, 26L, 26L,
		 26L, 26L, 26L, 27L
	};
	for (int i = 0; i < 16; ++i)
	{
		// shift
		c = ((c >> shifts1[i]) | (c << shifts2[i]));
		d = ((d >> shifts1[i]) | (d << shifts2[i]));
		c &= 0x0fffffffL;
		d &= 0x0fffffffL;

		// PC-2
        s = openssl_des_skb[0][(c) & 0x3f] |
            openssl_des_skb[1][((c >> 6L) & 0x03) | ((c >> 7L) & 0x3c)] |
            openssl_des_skb[2][((c >> 13L) & 0x0f) | ((c >> 14L) & 0x30)] |
            openssl_des_skb[3][((c >> 20L) & 0x01) | ((c >> 21L) & 0x06) |
                       ((c >> 22L) & 0x38)];
        t = openssl_des_skb[4][(d) & 0x3f] |
            openssl_des_skb[5][((d >> 7L) & 0x03) | ((d >> 8L) & 0x3c)] |
            openssl_des_skb[6][(d >> 15L) & 0x3f] |
            openssl_des_skb[7][((d >> 21L) & 0x0f) | ((d >> 22L) & 0x30)];

        /* table contained 0213 4657 */
        t2 = ((t << 16L) | (s & 0x0000ffffL)) & 0xffffffffL;
		t2 = MUGGLE_OPENSSL_DES_ROTATE(t2, 30) & 0xffffffffL;
		memcpy(&ks->sk[i].bytes[0], &t2, sizeof(t2));

        t2 = ((s >> 16L) | (t & 0xffff0000L));
        t2 = MUGGLE_OPENSSL_DES_ROTATE(t2, 26) & 0xffffffffL;
		memcpy(&ks->sk[i].bytes[4], &t2, sizeof(t2));
	}
}

/*
 * equal to openssl - DES_set_key_checked
 * @key: 64bit char array
 * RETURN:
 * 	return 0 if key parity is odd (correct)
 * 	return -1 if key parity error
 * 	return -2 if illegal weak key
 * */
int muggle_openssl_des_set_key_checked(unsigned char *key, struct muggle_des_subkeys *ks)
{
	if (!openssl_des_check_key_parity(key))
	{
		return -1;
	}
	if (openssl_des_is_weak_key(key))
	{
		return -2;
	}

	muggle_openssl_des_set_key_unchecked(key, ks);
	return 0;
}

void muggle_openssl_encrypt1(
	const muggle_64bit_block_t *input,
	const struct muggle_des_subkeys *ks,
	muggle_64bit_block_t *output)
{
	uint32_t l, r;

	r = input->u32.l;
	l = input->u32.h;

	MUGGLE_OPENSSL_DES_IP(r, l);

	r = MUGGLE_OPENSSL_DES_ROTATE(r, 29) & 0xffffffffL;
	l = MUGGLE_OPENSSL_DES_ROTATE(l, 29) & 0xffffffffL;

	const uint32_t *ks_u32 = (const uint32_t*)&ks->sk[0];

	MUGGLE_OPENSSL_D_ENCRYPT(l, r, ks_u32, 0);     /* 1 */
	MUGGLE_OPENSSL_D_ENCRYPT(r, l, ks_u32, 2);     /* 2 */
	MUGGLE_OPENSSL_D_ENCRYPT(l, r, ks_u32, 4);     /* 3 */
	MUGGLE_OPENSSL_D_ENCRYPT(r, l, ks_u32, 6);     /* 4 */
	MUGGLE_OPENSSL_D_ENCRYPT(l, r, ks_u32, 8);     /* 5 */
	MUGGLE_OPENSSL_D_ENCRYPT(r, l, ks_u32, 10);    /* 6 */
	MUGGLE_OPENSSL_D_ENCRYPT(l, r, ks_u32, 12);    /* 7 */
	MUGGLE_OPENSSL_D_ENCRYPT(r, l, ks_u32, 14);    /* 8 */
	MUGGLE_OPENSSL_D_ENCRYPT(l, r, ks_u32, 16);    /* 9 */
	MUGGLE_OPENSSL_D_ENCRYPT(r, l, ks_u32, 18);    /* 10 */
	MUGGLE_OPENSSL_D_ENCRYPT(l, r, ks_u32, 20);    /* 11 */
	MUGGLE_OPENSSL_D_ENCRYPT(r, l, ks_u32, 22);    /* 12 */
	MUGGLE_OPENSSL_D_ENCRYPT(l, r, ks_u32, 24);    /* 13 */
	MUGGLE_OPENSSL_D_ENCRYPT(r, l, ks_u32, 26);    /* 14 */
	MUGGLE_OPENSSL_D_ENCRYPT(l, r, ks_u32, 28);    /* 15 */
	MUGGLE_OPENSSL_D_ENCRYPT(r, l, ks_u32, 30);    /* 16 */

	l = MUGGLE_OPENSSL_DES_ROTATE(l, 3) & 0xffffffffL;
	r = MUGGLE_OPENSSL_DES_ROTATE(r, 3) & 0xffffffffL;

	MUGGLE_OPENSSL_DES_FP(r, l);

	output->u32.l = l;
	output->u32.h = r;
}

void muggle_openssl_encrypt2(
	const muggle_64bit_block_t *input,
	const struct muggle_des_subkeys *ks,
	muggle_64bit_block_t *output)
{
	uint32_t l, r;

	r = input->u32.l;
	l = input->u32.h;

	r = MUGGLE_OPENSSL_DES_ROTATE(r, 29) & 0xffffffffL;
	l = MUGGLE_OPENSSL_DES_ROTATE(l, 29) & 0xffffffffL;

	const uint32_t *ks_u32 = (const uint32_t*)&ks->sk[0];

	MUGGLE_OPENSSL_D_ENCRYPT(l, r, ks_u32, 0);     /* 1 */
	MUGGLE_OPENSSL_D_ENCRYPT(r, l, ks_u32, 2);     /* 2 */
	MUGGLE_OPENSSL_D_ENCRYPT(l, r, ks_u32, 4);     /* 3 */
	MUGGLE_OPENSSL_D_ENCRYPT(r, l, ks_u32, 6);     /* 4 */
	MUGGLE_OPENSSL_D_ENCRYPT(l, r, ks_u32, 8);     /* 5 */
	MUGGLE_OPENSSL_D_ENCRYPT(r, l, ks_u32, 10);    /* 6 */
	MUGGLE_OPENSSL_D_ENCRYPT(l, r, ks_u32, 12);    /* 7 */
	MUGGLE_OPENSSL_D_ENCRYPT(r, l, ks_u32, 14);    /* 8 */
	MUGGLE_OPENSSL_D_ENCRYPT(l, r, ks_u32, 16);    /* 9 */
	MUGGLE_OPENSSL_D_ENCRYPT(r, l, ks_u32, 18);    /* 10 */
	MUGGLE_OPENSSL_D_ENCRYPT(l, r, ks_u32, 20);    /* 11 */
	MUGGLE_OPENSSL_D_ENCRYPT(r, l, ks_u32, 22);    /* 12 */
	MUGGLE_OPENSSL_D_ENCRYPT(l, r, ks_u32, 24);    /* 13 */
	MUGGLE_OPENSSL_D_ENCRYPT(r, l, ks_u32, 26);    /* 14 */
	MUGGLE_OPENSSL_D_ENCRYPT(l, r, ks_u32, 28);    /* 15 */
	MUGGLE_OPENSSL_D_ENCRYPT(r, l, ks_u32, 30);    /* 16 */

	l = MUGGLE_OPENSSL_DES_ROTATE(l, 3) & 0xffffffffL;
	r = MUGGLE_OPENSSL_DES_ROTATE(r, 3) & 0xffffffffL;

	output->u32.l = l;
	output->u32.h = r;
}

void muggle_openssl_des_gen_subkeys(
	int op,
	const muggle_64bit_block_t *key,
	struct muggle_des_subkeys *subkeys)
{
	muggle_openssl_des_set_key_unchecked((unsigned char*)key->bytes, subkeys);
	if (op == MUGGLE_DECRYPT)
	{
		unsigned char bytes[8];
		for (int i = 0; i < 8; ++i)
		{
			memcpy(bytes, subkeys->sk[i].bytes, 8);
			memcpy(subkeys->sk[i].bytes, subkeys->sk[15-i].bytes, 8);
			memcpy(subkeys->sk[15-i].bytes, bytes, 8);
		}
	}
}

void muggle_openssl_des_crypt(
	const muggle_64bit_block_t *input,
	const struct muggle_des_subkeys *ks,
	muggle_64bit_block_t *output)
{
	muggle_openssl_encrypt1(input, ks, output);
}

void muggle_openssl_tdes_crypt(
	const muggle_64bit_block_t *input,
	const struct muggle_des_subkeys *ks1,
	const struct muggle_des_subkeys *ks2,
	const struct muggle_des_subkeys *ks3,
	muggle_64bit_block_t *output)
{
	uint32_t l, r;

	muggle_64bit_block_t input_block;
	MUGGLE_OPENSSL_C2L(&input->u32.l, input_block.u32.l);
	MUGGLE_OPENSSL_C2L(&input->u32.h, input_block.u32.h);

	l = input->u32.l;
	r = input->u32.h;

	MUGGLE_OPENSSL_DES_IP(l, r);

	input_block.u32.l = l;
	input_block.u32.h = r;

	muggle_openssl_encrypt2(&input_block, ks1, output);
	input_block.u64 = output->u64;

	muggle_openssl_encrypt2(&input_block, ks2, output);
	input_block.u64 = output->u64;

	muggle_openssl_encrypt2(&input_block, ks3, output);

	l = output->u32.l;
	r = output->u32.h;
	MUGGLE_OPENSSL_DES_FP(r, l);
	output->u32.l = l;
	output->u32.h = r;
}
