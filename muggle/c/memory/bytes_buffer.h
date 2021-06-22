/******************************************************************************
 *  @file         bytes_buffer.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-17
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec bytes buffer
 *****************************************************************************/
 
#ifndef MUGGLE_C_BYTES_BUFFER_H_
#define MUGGLE_C_BYTES_BUFFER_H_

#include "muggle/c/base/macro.h"
#include "muggle/c/base/atomic.h"
#include <stdbool.h>

EXTERN_C_BEGIN

/**
 * @brief bytes buffer
 *
 * use for write/read bytes of variable length, is not thread safe
 */
typedef struct muggle_bytes_buffer
{
	int c;
	int w;
	int r;
	int t;
	char *buffer;
}muggle_bytes_buffer_t;

/**
 * @brief initialize byte buffer
 *
 * @param bytes_buf  pointer to bytes buffer
 * @param capacity   init capacity
 *
 * @return if success return true, otherwise return false
 */
MUGGLE_C_EXPORT
bool muggle_bytes_buffer_init(muggle_bytes_buffer_t *bytes_buf, int capacity);

/**
 * @brief destroy byte buffer
 *
 * @param bytes_buf  pointer to bytes buffer
 */
MUGGLE_C_EXPORT
void muggle_bytes_buffer_destroy(muggle_bytes_buffer_t *bytes_buf);

/**
 * @brief get len of remain memory that used for write
 *
 * @param bytes_buf  pointer to bytes buffer
 *
 * @return remain memory that used for write
 */
MUGGLE_C_EXPORT
int muggle_bytes_buffer_writable(muggle_bytes_buffer_t *bytes_buf);

/**
 * @brief get len of bytes can be read
 *
 * @param bytes_buf pointer to bytes buffer
 *
 * @return readable len of bytes 
 */
MUGGLE_C_EXPORT
int muggle_bytes_buffer_readable(muggle_bytes_buffer_t *bytes_buf);

/**
 * @brief get contiguous len of bytes can be read
 *
 * @param bytes_buf pointer to bytes buffer
 *
 * @return contiguous len of bytes can be read
 */
MUGGLE_C_EXPORT
int muggle_bytes_buffer_contiguous_readable(muggle_bytes_buffer_t *bytes_buf);

/**
 * @brief fetch bytes from buffer without move read position
 *
 * @param bytes_buf pointer to bytes buffer
 * @param num_bytes number of bytes to fetch
 * @param dst       save fetched bytes into dst
 *
 * @return boolean
 */
MUGGLE_C_EXPORT
bool muggle_bytes_buffer_fetch(muggle_bytes_buffer_t *bytes_buf, int num_bytes, void *dst);

/**
 * @brief read bytes from buffer and move read position
 *
 * @param bytes_buf  pointer to bytes buffer
 * @param num_bytes  number of bytes to fetch
 * @param dst        save fetched bytes into dst
 *
 * @return boolean
 */
MUGGLE_C_EXPORT
bool muggle_bytes_buffer_read(muggle_bytes_buffer_t *bytes_buf, int num_bytes, void *dst);

/**
 * @brief write bytes into buffer and move write position
 *
 * @param bytes_buf  pointer to bytes buffer
 * @param num_bytes  number of bytes to write
 * @param src        address of bytes
 *
 * @return boolean
 */
MUGGLE_C_EXPORT
bool muggle_bytes_buffer_write(muggle_bytes_buffer_t *bytes_buf, int num_bytes, void *src);

/**
 * @brief find contiguous memory for write without move writer
 *
 * NOTE: usually use with muggle_bytes_buffer_writer_move
 *
 * @param bytes_buf  pointer to bytes buffer
 * @param num_bytes  number of required memory bytes
 *
 * @return
 *     - if has enough contiguous memory for write, return start of writer memory
 *     - if return NULL, mean have no enough contiguous memory for write
 */
MUGGLE_C_EXPORT
void* muggle_bytes_buffer_writer_fc(muggle_bytes_buffer_t *bytes_buf, int num_bytes);

/**
 * @brief move writer forward in contiguous memory
 *
 * NOTE: usually use with muggle_bytes_buffer_writer_fc
 *
 * @param bytes_buf  pointer to bytes buffer
 * @param num_bytes  move forward number bytes
 *
 * @return 
 *     if has enough contiguous memory for writer move forward (include after 
 *     jump), return true, otherwise return false
 */
MUGGLE_C_EXPORT
bool muggle_bytes_buffer_writer_move(muggle_bytes_buffer_t *bytes_buf, int num_bytes);

/**
 * @brief find contiguous memory for read without move reader
 *
 * NOTE: usually use with muggle_bytes_buffer_reader_move
 *
 * @param bytes_buf  pointer to bytes buffer
 * @param num_bytes  number of required memory bytes
 *
 * @return
 *     - if has enough contiguous memory for read, return start of read memory
 *     - if return null, mean have no enough contiguous memory for read
 */
MUGGLE_C_EXPORT
void* muggle_bytes_buffer_reader_fc(muggle_bytes_buffer_t *bytes_buf, int num_bytes);

/**
 * @brief move reader forward in contiguous memory
 *
 * NOTE: usually use with muggle_bytes_buffer_reader_fc
 *
 * @param bytes_buf  pointer to bytes buffer
 * @param num_bytes  move forward number bytes
 * 
 * @return
 *     if has enough contiguous memory for reader move forward (don't include after 
 *     jump), return true, otherwise return false
 */
MUGGLE_C_EXPORT
bool muggle_bytes_buffer_reader_move(muggle_bytes_buffer_t *bytes_buf, int num_bytes);

/**
 * @brief clear bytes in bytes buffer 
 *
 * @param bytes_buf  pointer to bytes buffer
 */
MUGGLE_C_EXPORT
void muggle_bytes_buffer_clear(muggle_bytes_buffer_t *bytes_buf);

EXTERN_C_END

#endif
