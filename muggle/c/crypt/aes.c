/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "aes.h"
#include <string.h>
#include "muggle/c/base/err.h"
#include "muggle/c/log/log.h"
#include "muggle/c/crypt/internal/internal_aes.h"

static const uint32_t s_aes_rcon[] = {
	0x01000000,0x02000000,0x04000000,0x08000000,
	0x10000000,0x20000000,0x40000000,0x80000000,
	0x1b000000,0x36000000,0x6c000000,0xd8000000,
	0xab000000,0x4d000000,0x9a000000
};

int muggle_aes_key_setup(const unsigned char *key, int bits, muggle_aes_sub_keys_t *sk)
{
	// variable's name is consistent with fips-197
	int Nb, Nr, Nk, i;

	if (key == NULL)
	{
		MUGGLE_ASSERT_MSG(key != NULL, "AES key setup with null key");
		return MUGGLE_ERR_NULL_PARAM;
	}

	if (sk == NULL)
	{
		MUGGLE_ASSERT_MSG(key != NULL, "AES key setup with null key schedule");
		return MUGGLE_ERR_NULL_PARAM;
	}

	Nb = 4;
	switch (bits)
	{
	case 128: 
	{
		Nr = 10;
		Nk = 4;
	}break;
	case 192:
	{
		Nr = 12;
		Nk = 6;
	}break;
	case 256:
	{
		Nr = 14;
		Nk = 8;
	}break;
	default:
	{
		MUGGLE_ASSERT_MSG(
			bits == 128 || bits == 192 || bits == 256,
			"AES key setup only support bit size: 128/192/256");
		return MUGGLE_ERR_CRYPT_KEY_SIZE;
	}
	}

	i = 0;
	sk->rounds = Nr;
	for (i = 0; i < Nk; i++)
	{
		sk->rd_key[i] =
			(key[4*i + 0] << 24) |
			(key[4*i + 1] << 16) |
			(key[4*i + 2] << 8) |
			(key[4*i + 3]);
	}

	for (i = Nk; i < Nb * (Nr + 1); i++)
	{
		uint32_t temp = sk->rd_key[i - 1];
#if MUGGLE_CRYPT_AES_DEBUG
		printf("Expansion key[%d]: temp=%08x\n", i, temp);
#endif
		if (i % Nk == 0)
		{
			// RotWord
			temp = muggle_aes_rot_word(temp);
#if MUGGLE_CRYPT_AES_DEBUG
			printf("Expansion key[%d]: After RotWord=%08x\n", i, temp);
#endif

			// SubWord
			temp = muggle_aes_sub_word(temp);
#if MUGGLE_CRYPT_AES_DEBUG
			printf("Expansion key[%d]: After SubWord=%08x\n", i, temp);
			printf("Expansion key[%d]: Rcon[i/Nk]=%08x\n", i, s_aes_rcon[(i-1)/Nk]); 
#endif

			// w[i]
			temp = temp ^ s_aes_rcon[(i-1)/Nk];

#if MUGGLE_CRYPT_AES_DEBUG
			printf("Expansion key[%d]: After XOR with Rcon=%08x\n", i, temp);
#endif
		}
		else if (Nk > 6 && (i % Nk == 4))
		{
			temp = muggle_aes_sub_word(temp);
#if MUGGLE_CRYPT_AES_DEBUG
			printf("Expansion key[%d]: After SubWord=%08x\n", i, temp);
#endif
		}

#if MUGGLE_CRYPT_AES_DEBUG
		printf("Expansion key[%d]: w[i-Nk]=%08x\n", i, sk->rd_key[i-Nk]);
#endif

		sk->rd_key[i] = sk->rd_key[i-Nk] ^ temp;

#if MUGGLE_CRYPT_AES_DEBUG
		printf("Expansion key[%d]: w[i]=%08x\n", i, sk->rd_key[i]);
#endif
	}

	return 0;
}

static int muggle_aes_encrypt(
	unsigned char *state,
	muggle_aes_sub_keys_t *sk)
{
	if (sk->rounds != 10 && sk->rounds != 12 && sk->rounds != 14)
	{
		MUGGLE_ASSERT_MSG(
			(sk->rounds == 10 || sk->rounds == 12 || sk->rounds == 14),
			"Invalid AES key schedule rounds"
		);
		return MUGGLE_ERR_INVALID_PARAM;
	}

#if MUGGLE_CRYPT_AES_DEBUG
	printf("AES encrypt[0]: Start of Round\n");
	muggle_output_hex(state, 16, 4);
	printf("AES encrypt[0]: Round Key Value\n");
	muggle_output_hex((unsigned char*)&sk->rd_key[0], 16, 4);
#endif

	muggle_aes_add_round_key(state, &sk->rd_key[0]);

	// round 1
#if MUGGLE_CRYPT_AES_DEBUG
	printf("AES encrypt[1]: Start of Round\n");
	muggle_output_hex(state, 16, 4);
#endif

	muggle_aes_sub_bytes(state);

#if MUGGLE_CRYPT_AES_DEBUG
	printf("AES encrypt[1]: After SubBytes\n");
	muggle_output_hex(state, 16, 4);
#endif

	muggle_aes_shift_rows(state);

#if MUGGLE_CRYPT_AES_DEBUG
	printf("AES encrypt[1]: After ShiftRows\n");
	muggle_output_hex(state, 16, 4);
#endif

	muggle_aes_mix_column(state);

#if MUGGLE_CRYPT_AES_DEBUG
	printf("AES encrypt[1]: After MixColumns\n");
	muggle_output_hex(state, 16, 4);
#endif

	muggle_aes_add_round_key(state, &sk->rd_key[4]);

#if MUGGLE_CRYPT_AES_DEBUG
	printf("AES encrypt[1]: Round Key Value\n");
	muggle_output_hex((unsigned char*)&sk->rd_key[4], 16, 4);
#endif

	// round 2 
	muggle_aes_sub_bytes(state);
	muggle_aes_shift_rows(state);
	muggle_aes_mix_column(state);
	muggle_aes_add_round_key(state, &sk->rd_key[8]);

	// round 3 
	muggle_aes_sub_bytes(state);
	muggle_aes_shift_rows(state);
	muggle_aes_mix_column(state);
	muggle_aes_add_round_key(state, &sk->rd_key[12]);

	// round 4 
	muggle_aes_sub_bytes(state);
	muggle_aes_shift_rows(state);
	muggle_aes_mix_column(state);
	muggle_aes_add_round_key(state, &sk->rd_key[16]);

	// round 5 
	muggle_aes_sub_bytes(state);
	muggle_aes_shift_rows(state);
	muggle_aes_mix_column(state);
	muggle_aes_add_round_key(state, &sk->rd_key[20]);

	// round 6 
	muggle_aes_sub_bytes(state);
	muggle_aes_shift_rows(state);
	muggle_aes_mix_column(state);
	muggle_aes_add_round_key(state, &sk->rd_key[24]);

	// round 7 
	muggle_aes_sub_bytes(state);
	muggle_aes_shift_rows(state);
	muggle_aes_mix_column(state);
	muggle_aes_add_round_key(state, &sk->rd_key[28]);

	// round 8 
	muggle_aes_sub_bytes(state);
	muggle_aes_shift_rows(state);
	muggle_aes_mix_column(state);
	muggle_aes_add_round_key(state, &sk->rd_key[32]);

	// round 9 
	muggle_aes_sub_bytes(state);
	muggle_aes_shift_rows(state);
	muggle_aes_mix_column(state);
	muggle_aes_add_round_key(state, &sk->rd_key[36]);

	if (sk->rounds == 10)
	{
		// round 10
#if MUGGLE_CRYPT_AES_DEBUG
		printf("AES encrypt[10]: Start of Round\n");
		muggle_output_hex(state, 16, 4);
#endif

		muggle_aes_sub_bytes(state);

#if MUGGLE_CRYPT_AES_DEBUG
		printf("AES encrypt[10]: After SubBytes\n");
		muggle_output_hex(state, 16, 4);
#endif

		muggle_aes_shift_rows(state);

#if MUGGLE_CRYPT_AES_DEBUG
		printf("AES encrypt[10]: After ShiftRows\n");
		muggle_output_hex(state, 16, 4);
		printf("AES encrypt[10]: Round Key Value\n");
		muggle_output_hex((unsigned char*)&sk->rd_key[40], 16, 4);
#endif

		muggle_aes_add_round_key(state, &sk->rd_key[40]);

#if MUGGLE_CRYPT_AES_DEBUG
		printf("AES encrypt[10]: After AddRoundKey\n");
		muggle_output_hex(state, 16, 4);
#endif
	}
	else
	{
		// round 10
		muggle_aes_sub_bytes(state);
		muggle_aes_shift_rows(state);
		muggle_aes_mix_column(state);
		muggle_aes_add_round_key(state, &sk->rd_key[40]);

		// round 11
		muggle_aes_sub_bytes(state);
		muggle_aes_shift_rows(state);
		muggle_aes_mix_column(state);
		muggle_aes_add_round_key(state, &sk->rd_key[44]);

		if (sk->rounds == 12)
		{
			// round 12
			muggle_aes_sub_bytes(state);
			muggle_aes_shift_rows(state);
			muggle_aes_add_round_key(state, &sk->rd_key[48]);
		}
		else if (sk->rounds == 14)
		{
			// round 12
			muggle_aes_sub_bytes(state);
			muggle_aes_shift_rows(state);
			muggle_aes_mix_column(state);
			muggle_aes_add_round_key(state, &sk->rd_key[48]);

			// round 13
			muggle_aes_sub_bytes(state);
			muggle_aes_shift_rows(state);
			muggle_aes_mix_column(state);
			muggle_aes_add_round_key(state, &sk->rd_key[52]);

			// round 14
			muggle_aes_sub_bytes(state);
			muggle_aes_shift_rows(state);
			muggle_aes_add_round_key(state, &sk->rd_key[56]);
		}
	}

	return 0;
}

static int muggle_aes_decrypt(
	unsigned char *state,
	muggle_aes_sub_keys_t *sk)
{
	if (sk->rounds != 10 && sk->rounds != 12 && sk->rounds != 14)
	{
		MUGGLE_ASSERT_MSG(
			(sk->rounds == 10 || sk->rounds == 12 || sk->rounds == 14),
			"Invalid AES key schedule rounds"
		);
		return MUGGLE_ERR_INVALID_PARAM;
	}

	if (sk->rounds > 10)
	{
		if (sk->rounds == 14)
		{
			// inv round 14
			muggle_aes_add_round_key(state, &sk->rd_key[56]);
			muggle_aes_inv_shift_rows(state);
			muggle_aes_inv_sub_bytes(state);

			// inv round 13
			muggle_aes_add_round_key(state, &sk->rd_key[52]);
			muggle_aes_inv_mix_column(state);
			muggle_aes_inv_shift_rows(state);
			muggle_aes_inv_sub_bytes(state);

			// inv round 12
			muggle_aes_add_round_key(state, &sk->rd_key[48]);
			muggle_aes_inv_mix_column(state);
			muggle_aes_inv_shift_rows(state);
			muggle_aes_inv_sub_bytes(state);
		}
		else
		{
			// inv round 12
			muggle_aes_add_round_key(state, &sk->rd_key[48]);
			muggle_aes_inv_shift_rows(state);
			muggle_aes_inv_sub_bytes(state);
		}

		// inv round 11
		muggle_aes_add_round_key(state, &sk->rd_key[44]);
		muggle_aes_inv_mix_column(state);
		muggle_aes_inv_shift_rows(state);
		muggle_aes_inv_sub_bytes(state);

		// inv round 10
		muggle_aes_add_round_key(state, &sk->rd_key[40]);
		muggle_aes_inv_mix_column(state);
		muggle_aes_inv_shift_rows(state);
		muggle_aes_inv_sub_bytes(state);
	}
	else
	{
		// inv round 10
#if MUGGLE_CRYPT_AES_DEBUG
		printf("AES decrypt[inv 10]: Start of Round\n");
		muggle_output_hex(state, 16, 4);
		printf("AES decrypt[inv 10]: Round Key Value\n");
		muggle_output_hex((unsigned char*)&sk->rd_key[40], 16, 4);
#endif

		muggle_aes_add_round_key(state, &sk->rd_key[40]);

#if MUGGLE_CRYPT_AES_DEBUG
		printf("AES decrypt[inv 10]: After AddRoundKey\n");
		muggle_output_hex(state, 16, 4);
#endif

		muggle_aes_inv_shift_rows(state);

#if MUGGLE_CRYPT_AES_DEBUG
		printf("AES decrypt[inv 10]: After ShiftRows\n");
		muggle_output_hex(state, 16, 4);
#endif

		muggle_aes_inv_sub_bytes(state);

#if MUGGLE_CRYPT_AES_DEBUG
		printf("AES encrypt[inv 10]: After SubBytes\n");
		muggle_output_hex(state, 16, 4);
#endif
	}

	// inv round 9
	muggle_aes_add_round_key(state, &sk->rd_key[36]);
	muggle_aes_inv_mix_column(state);
	muggle_aes_inv_shift_rows(state);
	muggle_aes_inv_sub_bytes(state);

	// inv round 8
	muggle_aes_add_round_key(state, &sk->rd_key[32]);
	muggle_aes_inv_mix_column(state);
	muggle_aes_inv_shift_rows(state);
	muggle_aes_inv_sub_bytes(state);

	// inv round 7
	muggle_aes_add_round_key(state, &sk->rd_key[28]);
	muggle_aes_inv_mix_column(state);
	muggle_aes_inv_shift_rows(state);
	muggle_aes_inv_sub_bytes(state);

	// inv round 6
	muggle_aes_add_round_key(state, &sk->rd_key[24]);
	muggle_aes_inv_mix_column(state);
	muggle_aes_inv_shift_rows(state);
	muggle_aes_inv_sub_bytes(state);

	// inv round 5
	muggle_aes_add_round_key(state, &sk->rd_key[20]);
	muggle_aes_inv_mix_column(state);
	muggle_aes_inv_shift_rows(state);
	muggle_aes_inv_sub_bytes(state);

	// inv round 4
	muggle_aes_add_round_key(state, &sk->rd_key[16]);
	muggle_aes_inv_mix_column(state);
	muggle_aes_inv_shift_rows(state);
	muggle_aes_inv_sub_bytes(state);

	// inv round 3
	muggle_aes_add_round_key(state, &sk->rd_key[12]);
	muggle_aes_inv_mix_column(state);
	muggle_aes_inv_shift_rows(state);
	muggle_aes_inv_sub_bytes(state);

	// inv round 2
	muggle_aes_add_round_key(state, &sk->rd_key[8]);
	muggle_aes_inv_mix_column(state);
	muggle_aes_inv_shift_rows(state);
	muggle_aes_inv_sub_bytes(state);

	// inv round 1
	muggle_aes_add_round_key(state, &sk->rd_key[4]);
	muggle_aes_inv_mix_column(state);
	muggle_aes_inv_shift_rows(state);
	muggle_aes_inv_sub_bytes(state);

	// inv round 0
	muggle_aes_add_round_key(state, &sk->rd_key[0]);

	return 0;
}

int muggle_aes_crypt(
	int op,
	const unsigned char *input,
	muggle_aes_sub_keys_t *sk,
	unsigned char *output)
{
	if (input == NULL)
	{
		MUGGLE_ASSERT_MSG(input != NULL, "AES crypt input plaintext is null");
		return MUGGLE_ERR_NULL_PARAM;
	}

	if (sk == NULL)
	{
		MUGGLE_ASSERT_MSG(sk != NULL, "AES crypt input key schedule null");
		return MUGGLE_ERR_NULL_PARAM;
	}

	if (output == NULL)
	{
		MUGGLE_ASSERT_MSG(sk != NULL, "AES crypt output ciphertext is null");
		return MUGGLE_ERR_NULL_PARAM;
	}

	int ret = 0;
	unsigned char state[16] = {
		input[0], input[4], input[8],  input[12],
		input[1], input[5], input[9],  input[13],
		input[2], input[6], input[10], input[14],
		input[3], input[7], input[11], input[15]
	};

	if (op == MUGGLE_ENCRYPT)
	{
		ret = muggle_aes_encrypt(state, sk);
	}
	else if (op == MUGGLE_DECRYPT)
	{
		ret = muggle_aes_decrypt(state, sk);
	}
	else
	{
		MUGGLE_ASSERT_MSG(op == MUGGLE_ENCRYPT || op == MUGGLE_DECRYPT, "Invalid AES crypt mode");
		ret = MUGGLE_ERR_INVALID_PARAM;
	}

	output[0]  = state[0];
	output[1]  = state[4];
	output[2]  = state[8];
	output[3]  = state[12];
	output[4]  = state[1];
	output[5]  = state[5];
	output[6]  = state[9];
	output[7]  = state[13];
	output[8]  = state[2];
	output[9]  = state[6];
	output[10] = state[10];
	output[11] = state[14];
	output[12] = state[3];
	output[13] = state[7];
	output[14] = state[11];
	output[15] = state[15];

	return ret;
}
