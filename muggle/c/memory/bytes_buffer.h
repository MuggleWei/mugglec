/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_BYTES_BUFFER_H_
#define MUGGLE_C_BYTES_BUFFER_H_

#include "muggle/c/base/macro.h"
#include "muggle/c/base/atomic.h"
#include <stdbool.h>

EXTERN_C_BEGIN

// muggle_bytes_buffer, use for write/read bytes of variable length, is not thread safe

typedef struct muggle_bytes_buffer
{
	int c;
	int w;
	int r;
	int t;
	char *buffer;
}muggle_bytes_buffer_t;

/*
 * initialize byte buffer
 * RETURN: if success return true, otherwise return false
 * */
MUGGLE_CC_EXPORT
bool muggle_bytes_buffer_init(muggle_bytes_buffer_t *bytes_buf, int capacity);

/*
 * destroy byte buffer
 * */
MUGGLE_CC_EXPORT
void muggle_bytes_buffer_destroy(muggle_bytes_buffer_t *bytes_buf);

/*
 * get len of remain memory use for write
 * */
MUGGLE_CC_EXPORT
int muggle_bytes_buffer_writable(muggle_bytes_buffer_t *bytes_buf);

/*
 * get len of bytes can be read
 * */
MUGGLE_CC_EXPORT
int muggle_bytes_buffer_readable(muggle_bytes_buffer_t *bytes_buf);

/*
 * get contiguous len of bytes can be read
 * */
MUGGLE_CC_EXPORT
int muggle_bytes_buffer_contiguous_readable(muggle_bytes_buffer_t *bytes_buf);

/*
 * fetch bytes from buffer without move read position
 * @num_bytes: number of bytes to fetch
 * @dst: save fetched bytes into dst
 * RETURN: true represent sucess, failed return false
 * */
MUGGLE_CC_EXPORT
bool muggle_bytes_buffer_fetch(muggle_bytes_buffer_t *bytes_buf, int num_bytes, void *dst);

/*
 * read bytes from buffer and move read position
 * @num_bytes: number of bytes to fetch
 * @dst: save fetched bytes into dst
 * RETURN: if success return true, otherwise return false
 * */
MUGGLE_CC_EXPORT
bool muggle_bytes_buffer_read(muggle_bytes_buffer_t *bytes_buf, int num_bytes, void *dst);

/*
 * write bytes into buffer and move write position
 * @num_bytes: number of bytes to write
 * @src: address of bytes
 * RETURN: if success return true, otherwise return false
 * */
MUGGLE_CC_EXPORT
bool muggle_bytes_buffer_write(muggle_bytes_buffer_t *bytes_buf, int num_bytes, void *src);

/*
 * find contiguous memory for write without move writer
 * @num_bytes: number of required memory bytes
 * RETURN: if has enough contiguous memory for write, return start of writer memory
 *         if return null, mean have no enough contiguous memory for write
 * NOTE: usually use with muggle_bytes_buffer_writer_move
 * */
MUGGLE_CC_EXPORT
void* muggle_bytes_buffer_writer_fc(muggle_bytes_buffer_t *bytes_buf, int num_bytes);

/*
 * move writer forward in contiguous memory
 * @num_bytes: move forward number bytes
 * RETURN: if has enough contiguous memory for writer move forward (include after 
 *         jump), return true, otherwise return false
 * NOTE: usually use with muggle_bytes_buffer_writer_fc
 * */
MUGGLE_CC_EXPORT
bool muggle_bytes_buffer_writer_move(muggle_bytes_buffer_t *bytes_buf, int num_bytes);

/*
 * find contiguous memory for read without move reader
 * @num_bytes: number of required memory bytes
 * RETURN: if has enough contiguous memory for read, return start of read memory
 *         if return null, mean have no enough contiguous memory for read
 * NOTE: usually use with muggle_bytes_buffer_reader_move
 * */
MUGGLE_CC_EXPORT
void* muggle_bytes_buffer_reader_fc(muggle_bytes_buffer_t *bytes_buf, int num_bytes);

/*
 * move reader forward in contiguous memory
 * @num_bytes: move forward number bytes
 * RETURN: if has enough contiguous memory for reader move forward (don't include after 
 *         jump), return true, otherwise return false
 * NOTE: usually use with muggle_bytes_buffer_reader_fc
 * */
MUGGLE_CC_EXPORT
bool muggle_bytes_buffer_reader_move(muggle_bytes_buffer_t *bytes_buf, int num_bytes);

/*
 * clear bytes in bytes buffer 
 * */
MUGGLE_CC_EXPORT
void muggle_bytes_buffer_clear(muggle_bytes_buffer_t *bytes_buf);

EXTERN_C_END

#endif
