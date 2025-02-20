#include "muggle/c/muggle_c.h"

void print_writer_fc(muggle_bytes_buffer_t *buf, int num_bytes)
{
	void *ptr = NULL;
	do {
		ptr = muggle_bytes_buffer_writer_fc(buf, num_bytes);
		LOG_INFO("find contiguous %d bytes, return 0x%llu", num_bytes, (void*)ptr);
		num_bytes++;
	} while (ptr);
}

int main()
{
	muggle_log_complicated_init(LOG_LEVEL_TRACE, -1, NULL);

	// data for write
	char data[32];
	memset(data, 0, sizeof(data));

	// init bytes buffer
	int capacity = 32;
	muggle_bytes_buffer_t buf;
	muggle_bytes_buffer_init(&buf, capacity);

	LOG_INFO("init bytes buffer, capacity: %d, writable: %d",
		capacity, muggle_bytes_buffer_writable(&buf));

	void *ptr = NULL;
	int num_bytes = 24;

	// find contiguous memory and write
	LOG_INFO("----------------------------------");
	ptr = muggle_bytes_buffer_writer_fc(&buf, num_bytes);
	MUGGLE_ASSERT(ptr != NULL);
	memset(ptr, 0, num_bytes);
	muggle_bytes_buffer_writer_move_n(&buf, ptr, num_bytes);
	LOG_INFO("write contiguous %d bytes", num_bytes);
	LOG_INFO("remain writable=%d, readable=%d, contiguous_readable=%d",
		muggle_bytes_buffer_writable(&buf),
		muggle_bytes_buffer_readable(&buf),
		muggle_bytes_buffer_contiguous_readable(&buf));

	print_writer_fc(&buf, 1);

	// read 8 bytes
	LOG_INFO("----------------------------------");
	num_bytes = 8;
	ptr = muggle_bytes_buffer_reader_fc(&buf, num_bytes);
	MUGGLE_ASSERT(ptr != NULL);
	muggle_bytes_buffer_reader_move(&buf, num_bytes);
	LOG_INFO("read %d bytes", num_bytes);
	LOG_INFO("remain writable=%d, readable=%d, contiguous_readable=%d",
		muggle_bytes_buffer_writable(&buf),
		muggle_bytes_buffer_readable(&buf),
		muggle_bytes_buffer_contiguous_readable(&buf));

	print_writer_fc(&buf, 1);

	// read 8 bytes again
	LOG_INFO("----------------------------------");
	num_bytes = 8;
	ptr = muggle_bytes_buffer_reader_fc(&buf, num_bytes);
	MUGGLE_ASSERT(ptr != NULL);
	muggle_bytes_buffer_reader_move(&buf, num_bytes);
	LOG_INFO("read %d bytes", num_bytes);
	LOG_INFO("remain writable=%d, readable=%d, contiguous_readable=%d",
		muggle_bytes_buffer_writable(&buf),
		muggle_bytes_buffer_readable(&buf),
		muggle_bytes_buffer_contiguous_readable(&buf));

	print_writer_fc(&buf, 1);

	// write 4 bytes
	LOG_INFO("----------------------------------");
	num_bytes = 4;
	muggle_bytes_buffer_write(&buf, num_bytes, data);
	LOG_INFO("write %d bytes", num_bytes);
	LOG_INFO("remain writable=%d, readable=%d, contiguous_readable=%d",
		muggle_bytes_buffer_writable(&buf),
		muggle_bytes_buffer_readable(&buf),
		muggle_bytes_buffer_contiguous_readable(&buf));

	print_writer_fc(&buf, 1);

	// write contiguous 8 bytes
	LOG_INFO("----------------------------------");
	num_bytes = 8;
	ptr = muggle_bytes_buffer_writer_fc(&buf, num_bytes);
	MUGGLE_ASSERT(ptr != NULL);
	memset(ptr, 0, num_bytes);
	muggle_bytes_buffer_writer_move_n(&buf, ptr, num_bytes);
	LOG_INFO("write contiguous %d bytes", num_bytes);
	LOG_INFO("remain writable=%d, readable=%d, contiguous_readable=%d",
		muggle_bytes_buffer_writable(&buf),
		muggle_bytes_buffer_readable(&buf),
		muggle_bytes_buffer_contiguous_readable(&buf));

	print_writer_fc(&buf, 1);

	// destroy bytes buffer
	muggle_bytes_buffer_destroy(&buf);

	return 0;
}
