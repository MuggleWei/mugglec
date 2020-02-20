#include "bytes_buffer.h"
#include <string.h>
#include <stdlib.h>
#include "muggle/c/log/log.h"

static inline
int muggle_bytes_buffer_contiguous_readable(muggle_bytes_buffer_t *bytes_buf)
{
	if (bytes_buf->write_pos >= bytes_buf->read_pos)
	{
		return bytes_buf->write_pos - bytes_buf->read_pos;
	}
	else
	{
		return bytes_buf->truncate_pos - bytes_buf->read_pos;
	}
}

static inline
int muggle_bytes_buffer_jump_readable(muggle_bytes_buffer_t *bytes_buf)
{
	if (bytes_buf->write_pos >= bytes_buf->read_pos)
	{
		return 0;
	}
	else
	{
		return bytes_buf->write_pos;
	}
}

static inline
int muggle_bytes_buffer_contiguous_writeable(muggle_bytes_buffer_t *bytes_buf)
{
	if (bytes_buf->write_pos >= bytes_buf->read_pos)
	{
		int cw = bytes_buf->capacity - bytes_buf->write_pos;
		if (bytes_buf->read_pos == 0)
		{
			cw -= 1;
		}
		return cw;
	}
	else
	{
		return bytes_buf->read_pos - bytes_buf->write_pos - 1;
	}
}

static inline
int muggle_bytes_buffer_jump_writeable(muggle_bytes_buffer_t *bytes_buf)
{
	if (bytes_buf->write_pos >= bytes_buf->read_pos)
	{
		int jw = bytes_buf->read_pos - 1;
		if (jw < 0)
		{
			jw = 0;
		}
		return jw;
	}
	else
	{
		return 0;
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
	bytes_buf->capacity = capacity;
	bytes_buf->truncate_pos = capacity;
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
	if (bytes_buf->write_pos >= bytes_buf->read_pos)
	{
		return bytes_buf->capacity - bytes_buf->write_pos + bytes_buf->read_pos - 1;
	}
	else
	{
		return bytes_buf->read_pos - bytes_buf->write_pos - 1;
	}
}

int muggle_bytes_buffer_readable(muggle_bytes_buffer_t *bytes_buf)
{
	if (bytes_buf->write_pos >= bytes_buf->read_pos)
	{
		return bytes_buf->write_pos - bytes_buf->read_pos;
	}
	else
	{
		return bytes_buf->truncate_pos - bytes_buf->read_pos + bytes_buf->write_pos;
	}
}

bool muggle_bytes_buffer_fetch(muggle_bytes_buffer_t *bytes_buf, int num_bytes, void *dst)
{
	int contiguous_readable = muggle_bytes_buffer_contiguous_readable(bytes_buf);
	if (contiguous_readable >= num_bytes)
	{
		memcpy(dst, bytes_buf->buffer + bytes_buf->read_pos, num_bytes);
		return true;
	}

	int jump_readable = muggle_bytes_buffer_jump_readable(bytes_buf);
	if (contiguous_readable + jump_readable < num_bytes)
	{
		return false;
	}

	memcpy(dst, bytes_buf->buffer + bytes_buf->read_pos, contiguous_readable);
	memcpy((char*)dst + contiguous_readable, bytes_buf->buffer, jump_readable);

	return true;
}

bool muggle_bytes_buffer_read(muggle_bytes_buffer_t *bytes_buf, int num_bytes, void *dst)
{
	int contiguous_readable = muggle_bytes_buffer_contiguous_readable(bytes_buf);
	if (contiguous_readable >= num_bytes)
	{
		memcpy(dst, bytes_buf->buffer + bytes_buf->read_pos, num_bytes);

		bytes_buf->read_pos += num_bytes;
		if (bytes_buf->read_pos == bytes_buf->truncate_pos)
		{
			bytes_buf->read_pos = 0;
		}

		muggle_bytes_buffer_refresh(bytes_buf);

		return true;
	}

	int jump_readable = muggle_bytes_buffer_jump_readable(bytes_buf);
	if (contiguous_readable + jump_readable < num_bytes)
	{
		return false;
	}

	memcpy(dst, bytes_buf->buffer + bytes_buf->read_pos, contiguous_readable);
	memcpy((char*)dst + contiguous_readable, bytes_buf->buffer, jump_readable);
	bytes_buf->read_pos = jump_readable;

	muggle_bytes_buffer_refresh(bytes_buf);

	return true;
}

bool muggle_bytes_buffer_write(muggle_bytes_buffer_t *bytes_buf, int num_bytes, void *src)
{
	int contiguous_writeable = muggle_bytes_buffer_contiguous_writeable(bytes_buf);
	if (contiguous_writeable >= num_bytes)
	{
		memcpy(bytes_buf->buffer + bytes_buf->write_pos, src, num_bytes);
		bytes_buf->write_pos += num_bytes;
		if (bytes_buf->truncate_pos < bytes_buf->write_pos)
		{
			bytes_buf->truncate_pos = bytes_buf->capacity;
		}
		if (bytes_buf->write_pos == bytes_buf->capacity)
		{
			bytes_buf->write_pos = 0;
		}
		return true;
	}

	int jump_writeable = muggle_bytes_buffer_jump_writeable(bytes_buf);
	if (contiguous_writeable + jump_writeable < num_bytes)
	{
		return false;
	}

	if (jump_writeable > num_bytes)
	{
		bytes_buf->truncate_pos = bytes_buf->write_pos;
		memcpy(bytes_buf->buffer, src, num_bytes);
		bytes_buf->write_pos = num_bytes;
	}
	else
	{
		bytes_buf->truncate_pos = bytes_buf->capacity;
		memcpy((char*)bytes_buf->buffer + bytes_buf->write_pos, src, num_bytes);
		int remain = num_bytes - contiguous_writeable;
		memcpy(bytes_buf->buffer, (char*)src + contiguous_writeable, remain);
		bytes_buf->write_pos = remain;
	}

	return true;
}

void* muggle_bytes_buffer_writer_get(muggle_bytes_buffer_t *bytes_buf, int num_bytes)
{
	int contiguous_writeable = muggle_bytes_buffer_contiguous_writeable(bytes_buf);

	char *ret = NULL;
	if (contiguous_writeable >= num_bytes)
	{
		ret = (char*)bytes_buf->buffer + bytes_buf->write_pos;
		bytes_buf->write_pos += num_bytes;
		return ret;
	}
	
	int jump_writeable = muggle_bytes_buffer_jump_writeable(bytes_buf);
	if (jump_writeable >= num_bytes)
	{
		ret = bytes_buf->buffer;
		bytes_buf->write_pos = num_bytes;
		return ret;
	}

	return (void*)ret;
}

void* muggle_bytes_buffer_reader_get(muggle_bytes_buffer_t *bytes_buf, int num_bytes)
{
	int contiguous_readable = muggle_bytes_buffer_contiguous_readable(bytes_buf);

	void *ret = NULL;
	if (contiguous_readable >= num_bytes)
	{
		ret = (char*)bytes_buf->buffer + bytes_buf->read_pos;
		bytes_buf->read_pos += num_bytes;
	}

	return ret;
}

void muggle_bytes_buffer_refresh(muggle_bytes_buffer_t *bytes_buf)
{
	if (bytes_buf->write_pos == bytes_buf->read_pos)
	{
		muggle_bytes_buffer_clear(bytes_buf);
	}
}

void muggle_bytes_buffer_clear(muggle_bytes_buffer_t *bytes_buf)
{
	bytes_buf->write_pos = 0;
	bytes_buf->read_pos = 0;
	bytes_buf->truncate_pos = bytes_buf->capacity;
}