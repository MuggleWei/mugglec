/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef TRANS_MESSAGE_H_
#define TRANS_MESSAGE_H_

#include "muggle_benchmark/muggle_benchmark.h"

#pragma pack(push, 1)

enum
{
	TRANS_PKG_ROUND   = 1000,
	PKG_PER_ROUND     = 1000,
	ROUND_INTERVAL_MS = 1,
};

enum
{
	MSG_TYPE_NULL = 0,
	MSG_TYPE_PKG,
	MSG_TYPE_END,
	MAX_MSG_TYPE,
};

struct pkg_header
{
	uint16_t little_endian;    // 1 - little endian, 0 - big endian
	uint16_t msg_type;         // MSG_TYPE_*
	uint32_t data_len;         // length of data
};

struct pkg
{
	struct pkg_header header;
	char placeholder[512 - sizeof(struct pkg_header)];
};

struct pkg_data 
{
	uint32_t idx;
	uint64_t sec;
	uint32_t nsec;
};

#pragma pack(pop)

void genPkgHeader(struct pkg_header *header);

void genPkgData(struct pkg_data *data, uint32_t idx);

void sendPkgs(muggle_socket_peer_t *peer);

#endif
