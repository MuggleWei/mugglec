#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"

#define TEST_BYTES_BUF_SPACE 16

void check_empty_status(muggle_bytes_buffer_t *bytes_buf, int capacity)
{
	char space[2 * TEST_BYTES_BUF_SPACE];
	int n = 0;
	void *ptr;
	bool ret = false;

	n = muggle_bytes_buffer_writable(bytes_buf);
	ASSERT_EQ(n, capacity - 1);

	n = muggle_bytes_buffer_readable(bytes_buf);
	ASSERT_EQ(n, 0);

	ret = muggle_bytes_buffer_fetch(bytes_buf, 1, space);
	ASSERT_FALSE(ret);

	ret = muggle_bytes_buffer_read(bytes_buf, 1, space);
	ASSERT_FALSE(ret);

	ret = muggle_bytes_buffer_write(bytes_buf, capacity, space);
	ASSERT_FALSE(ret);

	ptr = muggle_bytes_buffer_writer_fc(bytes_buf, capacity);
	ASSERT_TRUE(ptr == NULL);

	ptr = muggle_bytes_buffer_writer_fc(bytes_buf, capacity - 1);
	ASSERT_TRUE(ptr != NULL);

	ret = muggle_bytes_buffer_writer_move(bytes_buf, capacity);
	ASSERT_FALSE(ret);

	ptr = muggle_bytes_buffer_reader_fc(bytes_buf, 1);
	ASSERT_TRUE(ptr == NULL);

	ret = muggle_bytes_buffer_reader_move(bytes_buf, 1);
	ASSERT_FALSE(ret);
}

/*
 * build status: w greater than r, r not equal 0
 * @w: writer position
 * @r: reader position
 * */
void build_status_w_gt_r_ne_0(muggle_bytes_buffer_t *bytes_buf, int capacity, int w, int r)
{
	bool ret = false;

	ASSERT_GT(w, r);

	muggle_bytes_buffer_clear(bytes_buf);

	check_empty_status(bytes_buf, capacity);

	ret = muggle_bytes_buffer_writer_move(bytes_buf, w);
	ASSERT_TRUE(ret);

	ret = muggle_bytes_buffer_reader_move(bytes_buf, r);
	ASSERT_TRUE(ret);
}

/*
* build status: w greater than r, r not equal 0
* @w: writer position
* @r: reader position
* @t: truncate position
* */
void build_status_r_gt_w(muggle_bytes_buffer_t *bytes_buf, int capacity, int w, int r, int t)
{
	bytes_buf->w = w;
	bytes_buf->r = r;
	bytes_buf->t = t;
}

TEST(bytes_buffer, empty)
{
	int capacity = TEST_BYTES_BUF_SPACE + 1;
	muggle_bytes_buffer_t bytes_buf;
	bool ret = muggle_bytes_buffer_init(&bytes_buf, capacity);
	ASSERT_TRUE(ret);

	int n = 0;
	void *ptr = NULL;
	char space[2 * TEST_BYTES_BUF_SPACE];
	memset(space, 0, sizeof(space));
	
	check_empty_status(&bytes_buf, capacity);

	muggle_bytes_buffer_destroy(&bytes_buf);
}

TEST(bytes_buffer, w_ge_r0)
{
	int capacity = TEST_BYTES_BUF_SPACE + 1;
	muggle_bytes_buffer_t bytes_buf;
	bool ret = muggle_bytes_buffer_init(&bytes_buf, capacity);
	ASSERT_TRUE(ret);

	int n = 0;
	void *ptr = NULL;
	char space[2 * TEST_BYTES_BUF_SPACE];
	memset(space, 0, sizeof(space));

	// test write, writer find contiguous, writable and readable
	{
		// check empty status
		check_empty_status(&bytes_buf, capacity);

		// writer fetch
		ptr = muggle_bytes_buffer_writer_fc(&bytes_buf, capacity);
		ASSERT_TRUE(ptr == NULL);

		ptr = muggle_bytes_buffer_writer_fc(&bytes_buf, capacity - 1);
		ASSERT_TRUE(ptr != NULL);

		// writer move
		ret = muggle_bytes_buffer_writer_move(&bytes_buf, 4);
		ASSERT_TRUE(ret);

		// check status
		n = muggle_bytes_buffer_writable(&bytes_buf);
		ASSERT_EQ(n, capacity - 1 - 4);

		n = muggle_bytes_buffer_readable(&bytes_buf);
		ASSERT_EQ(n, 4);

		ptr = muggle_bytes_buffer_writer_fc(&bytes_buf, capacity - 1 - 4);
		ASSERT_TRUE(ptr != NULL);

		ptr = muggle_bytes_buffer_writer_fc(&bytes_buf, capacity - 1 - 4 + 1);
		ASSERT_TRUE(ptr == NULL);

		// write
		ret = muggle_bytes_buffer_write(&bytes_buf, 4, space);
		ASSERT_TRUE(ret);

		// check status
		n = muggle_bytes_buffer_writable(&bytes_buf);
		ASSERT_EQ(n, capacity - 1 - 2 * 4);

		n = muggle_bytes_buffer_readable(&bytes_buf);
		ASSERT_EQ(n, 2 * 4);

		ptr = muggle_bytes_buffer_writer_fc(&bytes_buf, capacity - 1 - 2 * 4);
		ASSERT_TRUE(ptr != NULL);

		ptr = muggle_bytes_buffer_writer_fc(&bytes_buf, capacity - 1 - 2 * 4 + 1);
		ASSERT_TRUE(ptr == NULL);
	}

	// test read
	muggle_bytes_buffer_clear(&bytes_buf);
	{
		// check empty status
		check_empty_status(&bytes_buf, capacity);

		// writer move
		ret = muggle_bytes_buffer_writer_move(&bytes_buf, 4);
		ASSERT_TRUE(ret);

		// read
		ret = muggle_bytes_buffer_read(&bytes_buf, 5, space);
		ASSERT_FALSE(ret);

		ret = muggle_bytes_buffer_read(&bytes_buf, 4, space);
		ASSERT_TRUE(ret);

		// check empty status
		check_empty_status(&bytes_buf, capacity);
	}

	// test reader find contiguous and reader move
	muggle_bytes_buffer_clear(&bytes_buf);
	{
		// check empty status
		check_empty_status(&bytes_buf, capacity);

		// writer move
		ret = muggle_bytes_buffer_writer_move(&bytes_buf, 4);
		ASSERT_TRUE(ret);

		// reader find contiguous
		ptr = muggle_bytes_buffer_reader_fc(&bytes_buf, 5);
		ASSERT_TRUE(ptr == NULL);

		ptr = muggle_bytes_buffer_reader_fc(&bytes_buf, 4);
		ASSERT_TRUE(ptr != NULL);

		// reader move
		ret = muggle_bytes_buffer_reader_move(&bytes_buf, 5);
		ASSERT_FALSE(ret);

		ret = muggle_bytes_buffer_reader_move(&bytes_buf, 4);
		ASSERT_TRUE(ret);

		// check empty status
		check_empty_status(&bytes_buf, capacity);
	}

	muggle_bytes_buffer_destroy(&bytes_buf);
}

TEST(bytes_buffer, w_ge_r_ne_0)
{
	int capacity = TEST_BYTES_BUF_SPACE + 1;
	muggle_bytes_buffer_t bytes_buf;
	bool ret = muggle_bytes_buffer_init(&bytes_buf, capacity);
	ASSERT_TRUE(ret);

	int n = 0;
	void *ptr = NULL;
	char space[2 * TEST_BYTES_BUF_SPACE];
	memset(space, 0, sizeof(space));
	for (int w = 2; w < capacity; ++w)
	{
		for (int r = 1; r < w; ++r)
		{
			build_status_w_gt_r_ne_0(&bytes_buf, capacity, w, r);

			int writable = muggle_bytes_buffer_writable(&bytes_buf);
			ASSERT_EQ(writable, capacity - 1 - w + r);

			int readable = muggle_bytes_buffer_readable(&bytes_buf);
			ASSERT_EQ(readable, w - r);

			if (writable > 0)
			{
				// writer
				int cw = capacity - w;
				int jw = r - 1;
				int x = cw > jw ? cw : jw;

				ptr = muggle_bytes_buffer_writer_fc(&bytes_buf, x + 1);
				ASSERT_TRUE(ptr == NULL);

				ptr = muggle_bytes_buffer_writer_fc(&bytes_buf, x);
				ASSERT_TRUE(ptr != NULL);
			}

			if (readable > 0)
			{
				// reader
				ptr = muggle_bytes_buffer_reader_fc(&bytes_buf, readable + 1);
				ASSERT_TRUE(ptr == NULL);

				ptr = muggle_bytes_buffer_reader_fc(&bytes_buf, readable);
				ASSERT_TRUE(ptr != NULL);

				for (int i = 0; i < readable; ++i)
				{
					*((char*)ptr + i) = (char)i;
				}

				// fetch
				for (int i = 0; i < sizeof(space); ++i)
				{
					space[i] = (char)255;
				}

				ret = muggle_bytes_buffer_fetch(&bytes_buf, readable + 1, space);
				ASSERT_FALSE(ret);

				ret = muggle_bytes_buffer_fetch(&bytes_buf, readable, space);
				ASSERT_TRUE(ret);

				for (int i = 0; i < sizeof(space); ++i)
				{
					if (i < readable)
					{
						ASSERT_EQ(space[i], (char)i);
					}
					else
					{
						ASSERT_EQ(space[i], (char)255);
					}
				}

				// read
				for (int i = 0; i < sizeof(space); ++i)
				{
					space[i] = (char)255;
				}

				ret = muggle_bytes_buffer_read(&bytes_buf, readable + 1, space);
				ASSERT_FALSE(ret);

				ret = muggle_bytes_buffer_read(&bytes_buf, readable, space);
				ASSERT_TRUE(ret);

				for (int i = 0; i < sizeof(space); ++i)
				{
					if (i < readable)
					{
						ASSERT_EQ(space[i], (char)i);
					}
					else
					{
						ASSERT_EQ(space[i], (char)255);
					}
				}

				// check empty status
				check_empty_status(&bytes_buf, capacity);
			}
		}
	}

	muggle_bytes_buffer_destroy(&bytes_buf);
}

TEST(bytes_buffer, r_gt_w)
{
	int capacity = TEST_BYTES_BUF_SPACE + 1;
	muggle_bytes_buffer_t bytes_buf;
	bool ret = muggle_bytes_buffer_init(&bytes_buf, capacity);
	ASSERT_TRUE(ret);

	int n = 0;
	void *ptr = NULL;
	char space[2 * TEST_BYTES_BUF_SPACE];
	memset(space, 0, sizeof(space));

	for (int r = 1; r < capacity; ++r)
	{
		for (int w = 0; w < r; ++w)
		{
			for (int t = r; t < capacity; ++t)
			{
				build_status_r_gt_w(&bytes_buf, capacity, w, r, t);

				int writable = muggle_bytes_buffer_writable(&bytes_buf);
				ASSERT_EQ(writable, r - w - 1);

				int readable = muggle_bytes_buffer_readable(&bytes_buf);
				ASSERT_EQ(readable, t - r + w);

				if (writable > 0)
				{
					// writer
					int cw = r - w - 1;

					ptr = muggle_bytes_buffer_writer_fc(&bytes_buf, cw + 1);
					ASSERT_TRUE(ptr == NULL);

					ptr = muggle_bytes_buffer_writer_fc(&bytes_buf, cw);
					ASSERT_TRUE(ptr != NULL);
				}

				if (readable > 0)
				{
					// reader
					int cw = t - r;
					int jr = w;

					ptr = muggle_bytes_buffer_reader_fc(&bytes_buf, cw + 1);
					ASSERT_TRUE(ptr == NULL);

					ptr = muggle_bytes_buffer_reader_fc(&bytes_buf, cw);
					ASSERT_TRUE(ptr != NULL);

					for (int i = 0; i < readable; ++i)
					{
						if (i < cw)
						{
							*((char*)ptr + i) = (char)i;
						}
						else
						{
							bytes_buf.buffer[i - cw] = (char)i;
						}
					}

					// fetch
					for (int i = 0; i < sizeof(space); ++i)
					{
						space[i] = (char)255;
					}

					ret = muggle_bytes_buffer_fetch(&bytes_buf, readable + 1, space);
					ASSERT_FALSE(ret);

					ret = muggle_bytes_buffer_fetch(&bytes_buf, readable, space);
					ASSERT_TRUE(ret);

					for (int i = 0; i < sizeof(space); ++i)
					{
						if (i < readable)
						{
							ASSERT_EQ(space[i], (char)i);
						}
						else
						{
							if (space[i] != (char)255)
							{
								printf("fuck");
							}
							ASSERT_EQ(space[i], (char)255);
						}
					}

					// read
					for (int i = 0; i < sizeof(space); ++i)
					{
						space[i] = (char)255;
					}

					ret = muggle_bytes_buffer_read(&bytes_buf, readable + 1, space);
					ASSERT_FALSE(ret);

					ret = muggle_bytes_buffer_read(&bytes_buf, readable, space);
					ASSERT_TRUE(ret);

					for (int i = 0; i < sizeof(space); ++i)
					{
						if (i < readable)
						{
							ASSERT_EQ(space[i], (char)i);
						}
						else
						{
							ASSERT_EQ(space[i], (char)255);
						}
					}

					// check empty status
					check_empty_status(&bytes_buf, capacity);
				}
			}
		}
	}

	muggle_bytes_buffer_destroy(&bytes_buf);
}

TEST(bytes_buffer, truncate)
{
	int capacity = TEST_BYTES_BUF_SPACE + 1;
	muggle_bytes_buffer_t bytes_buf;
	bool ret = muggle_bytes_buffer_init(&bytes_buf, capacity);
	ASSERT_TRUE(ret);

	int n = 0;
	void *ptr = NULL;
	char space[2 * TEST_BYTES_BUF_SPACE];
	for (int i = 0; i < sizeof(space); ++i)
	{
		space[i] = (char)255;
	}

	for (int w = 2; w < capacity; ++w)
	{
		for (int r = 1; r < w; ++r)
		{
			int cw = capacity - w;
			if (cw < r - 1)
			{
				build_status_w_gt_r_ne_0(&bytes_buf, capacity, w, r);

				memset(bytes_buf.buffer, 0, bytes_buf.c);
				muggle_bytes_buffer_write(&bytes_buf, cw + 1, space);

				ASSERT_EQ(bytes_buf.t, w);
				for (int i = 0; i < bytes_buf.c; ++i)
				{
					if (i < cw + 1)
					{
						ASSERT_EQ(bytes_buf.buffer[i], (char)255);
					}
					else
					{
						ASSERT_EQ(bytes_buf.buffer[i], (char)0);
					}
				}
			}
		}
	}

	muggle_bytes_buffer_destroy(&bytes_buf);
}