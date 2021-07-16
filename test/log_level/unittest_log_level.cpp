#include "gtest/gtest.h"
#include "muggle/c/log/log_level.h"
#include "muggle/c/muggle_c.h"

TEST(log, log_level)
{
	ASSERT_STREQ(muggle_log_level_to_str(MUGGLE_LOG_LEVEL_TRACE), "TRACE");
	ASSERT_STREQ(muggle_log_level_to_str(MUGGLE_LOG_LEVEL_DEBUG), "DEBUG");
	ASSERT_STREQ(muggle_log_level_to_str(MUGGLE_LOG_LEVEL_INFO), "INFO");
	ASSERT_STREQ(muggle_log_level_to_str(MUGGLE_LOG_LEVEL_WARNING), "WARNING");
	ASSERT_STREQ(muggle_log_level_to_str(MUGGLE_LOG_LEVEL_WARN), "WARNING");
	ASSERT_STREQ(muggle_log_level_to_str(MUGGLE_LOG_LEVEL_ERROR), "ERROR");
	ASSERT_STREQ(muggle_log_level_to_str(MUGGLE_LOG_LEVEL_FATAL), "FATAL");
}
