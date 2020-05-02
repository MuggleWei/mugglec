/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "aes.h"
#include "muggle/c/base/err.h"
#include "muggle/c/log/log.h"

int muggle_aes_key_setup(const unsigned char *key, int bits, muggle_aes_sub_keys_t *sk)
{
	int num_rounds;

	if (key == NULL || sk == NULL)
	{
		return MUGGLE_ERR_NULL_PARAM;
	}

	switch (bits)
	{
	case 128:
		{
			sk->rounds = 10;
		}break;
	case 192:
		{
			sk->rounds = 12;
		}break;
	case 256:
		{
			sk->rounds = 14;
		}break;
	default:
		{
			return MUGGLE_ERR_CRYPT_KEY_SIZE;
		};
	}

	// TODO:

	return 0;
}
