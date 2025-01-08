/******************************************************************************
 *  @file         socket_evloop_pipe.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2024-11-27
 *  @copyright    Copyright 2024 Muggle Wei
 *  @license      MIT License
 *  @brief        socket event loop pipe
 *
 *  NOTE:
 *    - User must gurantee only one reader read event pipe at the same time
 *    - When event pipe full, write will block until success or pipe close
 *    - When event pipe empty, read will return NULL immediately
 *****************************************************************************/

#ifndef MUGGLE_C_SOCKET_EVLOOP_PIPE_H_
#define MUGGLE_C_SOCKET_EVLOOP_PIPE_H_

#include "muggle/c/base/macro.h"
#include "muggle/c/net/socket_context.h"
#include "muggle/c/sync/spinlock.h"
#include <stdbool.h>

EXTERN_C_BEGIN

typedef struct {
	muggle_socket_context_t ctx[2];
	muggle_spinlock_t lock;
} muggle_socket_evloop_pipe_t;

/**
 * @brief socket event loop pipe init
 *
 * @param ev_pipe  socket event loop pipe
 *
 * @return
 *     0 - success
 *     otherwise - failed
 */
MUGGLE_C_EXPORT
int muggle_socket_evloop_pipe_init(muggle_socket_evloop_pipe_t *ev_pipe);

/**
 * @brief destroy socket event loop pipe
 *
 * @param ev_pipe  socket event loop pipe
 */
MUGGLE_C_EXPORT
void muggle_socket_evloop_pipe_destroy(muggle_socket_evloop_pipe_t *ev_pipe);

/**
 * @brief socket event loop pipe write
 *
 * @param ev_pipe  socket event loop pipe
 * @param data     push data pointer
 *
 * @return
 *     0 - success
 *     otherwise - failed
 */
MUGGLE_C_EXPORT
bool muggle_socket_evloop_pipe_write(muggle_socket_evloop_pipe_t *ev_pipe,
									void *data);

/**
 * @brief socket event loop pipe read
 *
 * @param ev_pipe  socket event loop pipe
 *
 * @return  data that pipe write
 */
MUGGLE_C_EXPORT
void *muggle_socket_evloop_pipe_read(muggle_socket_evloop_pipe_t *ev_pipe);

/**
 * @brief get writer of pipe
 *
 * @param ev_pipe  socket event loop pipe
 *
 * @return  writer of pipe
 */
MUGGLE_C_EXPORT
muggle_socket_context_t *
muggle_socket_evloop_pipe_get_writer(muggle_socket_evloop_pipe_t *ev_pipe);

/**
 * @brief get reader of pipe
 *
 * @param ev_pipe  socket event loop pipe
 *
 * @return  reader of pipe
 */
MUGGLE_C_EXPORT
muggle_socket_context_t *
muggle_socket_evloop_pipe_get_reader(muggle_socket_evloop_pipe_t *ev_pipe);

EXTERN_C_END

#endif // !MUGGLE_C_SOCKET_EVLOOP_PIPE_H_
