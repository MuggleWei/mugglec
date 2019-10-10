#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"

TEST(log, fmt)
{
	int fmt_flag;
	char buf[4096];
	muggle_log_fmt_arg_t arg;
	int num_write;

	memset(&arg, 0, sizeof(arg));
	arg.level = 0;
	arg.line = 666;
	arg.file = "log_fmt_test_file";
	arg.func = "log_fmt_test_func";
	arg.tid = muggle_thread_current_id();

	fmt_flag = 0;
	num_write = muggle_log_fmt_gen(fmt_flag, &arg, "hello", buf, sizeof(buf));
	EXPECT_GT(num_write, 0);
	EXPECT_STREQ(buf, " - hello\n");

	fmt_flag = MUGGLE_LOG_FMT_LEVEL;
	num_write = muggle_log_fmt_gen(fmt_flag, &arg, "hello", buf, sizeof(buf));
	EXPECT_GT(num_write, 0);
	EXPECT_STREQ(buf, " - hello\n");

	fmt_flag = MUGGLE_LOG_FMT_LEVEL;
	arg.level = MUGGLE_LOG_LEVEL_INFO;
	num_write = muggle_log_fmt_gen(fmt_flag, &arg, "hello", buf, sizeof(buf));
	EXPECT_GT(num_write, 0);
	EXPECT_STREQ(buf, "<L>INFO| - hello\n");

	fmt_flag = MUGGLE_LOG_FMT_FILE;
	num_write = muggle_log_fmt_gen(fmt_flag, &arg, "hello", buf, sizeof(buf));
	EXPECT_GT(num_write, 0);
	EXPECT_STREQ(buf, "<F>log_fmt_test_file:666| - hello\n");

	fmt_flag = MUGGLE_LOG_FMT_FUNC;
	num_write = muggle_log_fmt_gen(fmt_flag, &arg, "hello", buf, sizeof(buf));
	EXPECT_GT(num_write, 0);
	EXPECT_STREQ(buf, "<f>log_fmt_test_func| - hello\n");

	fmt_flag = MUGGLE_LOG_FMT_TIME;
	num_write = muggle_log_fmt_gen(fmt_flag, &arg, "hello", buf, sizeof(buf));
	EXPECT_GT(num_write, 0);
	EXPECT_TRUE(muggle_str_startswith(buf, "<T>"));
	EXPECT_TRUE(muggle_str_endswith(buf, "| - hello\n"));

	fmt_flag = MUGGLE_LOG_FMT_THREAD;
	num_write = muggle_log_fmt_gen(fmt_flag, &arg, "hello", buf, sizeof(buf));
	EXPECT_GT(num_write, 0);
	EXPECT_TRUE(muggle_str_startswith(buf, "<t>"));
	EXPECT_TRUE(muggle_str_endswith(buf, "| - hello\n"));

	fmt_flag = 
		MUGGLE_LOG_FMT_LEVEL |
		MUGGLE_LOG_FMT_FILE |
		MUGGLE_LOG_FMT_FUNC |
		MUGGLE_LOG_FMT_TIME;
	arg.level = MUGGLE_LOG_LEVEL_WARNING;
	num_write = muggle_log_fmt_gen(fmt_flag, &arg, "hello", buf, sizeof(buf));
	EXPECT_GT(num_write, 0);
	EXPECT_TRUE(muggle_str_startswith(buf, "<L>WARNING|<F>log_fmt_test_file:666|<f>log_fmt_test_func|<T>"));
	EXPECT_TRUE(muggle_str_endswith(buf, "| - hello\n"));
}
