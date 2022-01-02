/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef CALLBACKS_H_
#define CALLBACKS_H_

#include "trans_message.h"

// message callback functions
typedef int (*fn_on_pkg)(muggle_socket_peer_t *peer, struct pkg *msg);

extern int       g_pkg_cnt;
extern fn_on_pkg g_callbacks[MAX_MSG_TYPE];

extern muggle_benchmark_block_t *g_blocks;

/****************** report ******************/
void init_report();
void gen_report(const char *name);
void gen_benchmark_report(const char *name, muggle_benchmark_block_t *block, int cnt);

/****************** message callbacks ******************/
void register_callbacks();

int on_msg(muggle_socket_peer_t *peer, struct pkg *msg);

int on_pkg(muggle_socket_peer_t *peer, struct pkg *msg);
int on_end(muggle_socket_peer_t *peer, struct pkg *msg);

#endif
