/******************************************************************************
 *  @file         bytes_buffer.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-17
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec bytes buffer
 *****************************************************************************/
 
#include "bytes_buffer.h"
#include <string.h>
#include <stdlib.h>
#include "muggle/c/log/log.h"

/*

variables:
	w: write position
	r: read position
	c: capacity
	t: truncate position

	cw: contiguous writable
	cr: contiguous readable
	jw: jump writable
	jr: jump readable
	tw: total writable = cw + jw
	tr: total readable = cr + jr

total 3 status:
	w >= r and r != 0:
		cw = c - w
		cr = w - r
		jw = r - 1
		jr = 0

	w >= r and r == 0:
		cw = c - w - 1
		cr = w - r
		jw = 0
		jr = 0

	r > w:
		cw = r - w - 1
		cr = t - r
		jw = 0
		jr = w

*/

static inline
int muggle_bytes_buffer_contiguous_writable(muggle_bytes_buffer_t *bytes_buf)
{
	if (bytes_buf->w >= bytes_buf->r)
	{
		if (bytes_buf->r != 0)
		{
			return bytes_buf->c - bytes_buf->w;
		}
		else
		{
			return bytes_buf->c - bytes_buf->w - 1;
		}
	}
	else
	{
		return bytes_buf->r - bytes_buf->w - 1;
	}
}

static inline
int muggle_bytes_buffer_jump_writable(muggle_bytes_buffer_t *bytes_buf)
{
	if (bytes_buf->w >= bytes_buf->r)
	{
		if (bytes_buf->r != 0)
		{
			return bytes_buf->r - 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
}

static inline
int muggle_bytes_buffer_jump_readable(muggle_bytes_buffer_t *bytes_buf)
{
	if (bytes_buf->w >= bytes_buf->r)
	{
		return 0;
	}
	else
	{
		return bytes_buf->w;
	}
}

/*
 * refresh bytes buffer, try to maximize contiguous memory
 * */
static inline
void muggle_bytes_buffer_refresh(muggle_bytes_buffer_t *bytes_buf)
{
	if (bytes_buf->w == bytes_buf->r)
	{
		muggle_bytes_buffer_clear(bytes_buf);
	}
}

bool muggle_bytes_buffer_init(muggle_bytes_buffer_t *bytes_buf, int capacity)
{
	memset(bytes_buf, 0, sizeof(muggle_bytes_buffer_t));
	bytes_buf->buffer = (char*)malloc(capacity);
	if (bytes_buf->buffer == NULL)
	{
		return false;
	}
	bytes_buf->c = capacity;
	bytes_buf->t = capacity;
	return true;
}

void muggle_bytes_buffer_destroy(muggle_bytes_buffer_t *bytes_buf)
{
	if (bytes_buf->buffer)
	{
		free(bytes_buf->buffer);
		bytes_buf->buffer = NULL;
	}
}

int muggle_bytes_buffer_writable(muggle_bytes_buffer_t *bytes_buf)
{
	int cw = muggle_bytes_buffer_contiguous_writable(bytes_buf);
	int jw = muggle_bytes_buffer_jump_writable(bytes_buf);
	return cw + jw;
}

int muggle_bytes_buffer_readable(muggle_bytes_buffer_t *bytes_buf)
{
	int cr = muggle_bytes_buffer_contiguous_readable(bytes_buf);
	int jr = muggle_bytes_buffer_jump_readable(bytes_buf);
	return cr + jr;
}

int muggle_bytes_buffer_contiguous_readable(muggle_bytes_buffer_t *bytes_buf)
{
	if (bytes_buf->w >= bytes_buf->r)
	{
		return bytes_buf->w - bytes_buf->r;
	}
	else
	{
		return bytes_buf->t - bytes_buf->r;
	}
}

bool muggle_bytes_buffer_fetch(muggle_bytes_buffer_t *bytes_buf, int num_bytes, void *dst)
{
	int cr = muggle_bytes_buffer_contiguous_readable(bytes_buf);
	if (cr >= num_bytes)
	{
		memcpy(dst, bytes_buf->buffer + bytes_buf->r, num_bytes);
		return true;
	}

	int jr = muggle_bytes_buffer_jump_readable(bytes_buf);
	if (cr + jr < num_bytes)
	{
		return false;
	}

	if (cr > 0)
	{
		memcpy(dst, bytes_buf->buffer + bytes_buf->r, cr);
	}
	int remain = num_bytes - cr;
	memcpy((char*)dst + cr, bytes_buf->buffer, remain);

	return true;
}

bool muggle_bytes_buffer_read(muggle_bytes_buffer_t *bytes_buf, int num_bytes, void *dst)
{
	int cr = muggle_bytes_buffer_contiguous_readable(bytes_buf);
	if (cr >= num_bytes)
	{
		memcpy(dst, bytes_buf->buffer + bytes_buf->r, num_bytes);

		bytes_buf->r += num_bytes;
		if (bytes_buf->r == bytes_buf->t)
		{
			bytes_buf->r = 0;
		}

		muggle_bytes_buffer_refresh(bytes_buf);

		return true;
	}

	int jr = muggle_bytes_buffer_jump_readable(bytes_buf);
	if (cr + jr < num_bytes)
	{
		return false;
	}

	if (cr > 0)
	{
		memcpy(dst, bytes_buf->buffer + bytes_buf->r, cr);
	}
	int remain = num_bytes - cr;
	memcpy((char*)dst + cr, bytes_buf->buffer, remain);
	bytes_buf->r = remain;

	muggle_bytes_buffer_refresh(bytes_buf);

	return true;
}

bool muggle_bytes_buffer_write(muggle_bytes_buffer_t *bytes_buf, int num_bytes, void *src)
{
	int cw = muggle_bytes_buffer_contiguous_writable(bytes_buf);
	if (cw >= num_bytes)
	{
		memcpy(bytes_buf->buffer + bytes_buf->w, src, num_bytes);
		bytes_buf->w += num_bytes;
		if (bytes_buf->t < bytes_buf->w)
		{
			bytes_buf->t = bytes_buf->c;
		}
		if (bytes_buf->w == bytes_buf->c)
		{
			bytes_buf->w = 0;
		}
		return true;
	}

	int jw = muggle_bytes_buffer_jump_writable(bytes_buf);
	if (cw + jw < num_bytes)
	{
		return false;
	}

	if (jw >= num_bytes)
	{
		bytes_buf->t = bytes_buf->w;
		memcpy(bytes_buf->buffer, src, num_bytes);
		bytes_buf->w = num_bytes;
	}
	else
	{
		bytes_buf->t = bytes_buf->c;

		if (cw > 0)
		{
			memcpy((char*)bytes_buf->buffer + bytes_buf->w, src, cw);
		}
		int remain = num_bytes - cw;
		memcpy(bytes_buf->buffer, (char*)src + cw, remain);
		bytes_buf->w = remain;
	}

	return true;
}

void* muggle_bytes_buffer_writer_fc(muggle_bytes_buffer_t *bytes_buf, int num_bytes)
{
	int cw = muggle_bytes_buffer_contiguous_writable(bytes_buf);
	if (cw >= num_bytes)
	{
		return bytes_buf->buffer + bytes_buf->w;
	}
	else
	{
		int jw = muggle_bytes_buffer_jump_writable(bytes_buf);
		if (jw >= num_bytes)
		{
			return bytes_buf->buffer;
		}
		else
		{
			return NULL;
		}
	}
}

bool muggle_bytes_buffer_writer_move(muggle_bytes_buffer_t *bytes_buf, int num_bytes)
{
	int cw = muggle_bytes_buffer_contiguous_writable(bytes_buf);
	if (cw >= num_bytes)
	{
		bytes_buf->w += num_bytes;
		if (bytes_buf->t < bytes_buf->w)
		{
			bytes_buf->t = bytes_buf->c;
		}
		if (bytes_buf->w == bytes_buf->c)
		{
			bytes_buf->w = 0;
		}

		return true;
	}
	else
	{
		int jw = muggle_bytes_buffer_jump_writable(bytes_buf);
		if (jw >= num_bytes)
		{
			bytes_buf->t = bytes_buf->w;
			bytes_buf->w = num_bytes;
			return true;
		}
		else
		{
			return false;
		}
	}
}

void* muggle_bytes_buffer_reader_fc(muggle_bytes_buffer_t *bytes_buf, int num_bytes)
{
	int cr = muggle_bytes_buffer_contiguous_readable(bytes_buf);
	if (cr >= num_bytes)
	{
		return (char*)bytes_buf->buffer + bytes_buf->r;
	}
	else
	{
		return NULL;
	}
}

bool muggle_bytes_buffer_reader_move(muggle_bytes_buffer_t *bytes_buf, int num_bytes)
{
	int cr = muggle_bytes_buffer_contiguous_readable(bytes_buf);
	if (cr >= num_bytes)
	{
		bytes_buf->r += num_bytes;
		muggle_bytes_buffer_refresh(bytes_buf);
		return true;
	}
	else
	{
		return false;
	}
}

void muggle_bytes_buffer_clear(muggle_bytes_buffer_t *bytes_buf)
{
	bytes_buf->w = 0;
	bytes_buf->r = 0;
	bytes_buf->t = bytes_buf->c;
}
