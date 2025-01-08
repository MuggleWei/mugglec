#include "gtest/gtest.h"
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

int customize_log_fmt_func(const muggle_log_msg_t *msg, char *buf, size_t bufsize)
{
	const char *level = muggle_log_level_to_str(msg->level);

	char filename[MUGGLE_MAX_PATH];
	muggle_path_basename(msg->src_loc.file, filename, sizeof(filename));

	const char *payload = "";
	if (msg->payload)
	{
		payload = msg->payload;
	}

	return (int)snprintf(buf, bufsize,
		"%s|%s:%u|%llu.%06d|%llu - %s\n",
		level,
		filename, (unsigned int)msg->src_loc.line,
		(unsigned long long)msg->ts.tv_sec,
		(int)msg->ts.tv_nsec / 1000,
		(unsigned long long)msg->tid,
		payload);
}

TEST(log, log_fmt)
{
	muggle_log_fmt_t formatter = {
		MUGGLE_LOG_FMT_ALL,
		customize_log_fmt_func
	};

	muggle_log_msg_t msg = {
		MUGGLE_LOG_LEVEL_INFO,
		{0, 0},
		0,
		{__FILE__, __LINE__, __FUNCTION__},
		"log formatter"
	};
	muggle_realtime_get(msg.ts);
	msg.tid = muggle_thread_current_readable_id();

	char buf[MUGGLE_LOG_MSG_MAX_LEN];
	formatter.fmt_func(&msg, buf, sizeof(buf));

	char expect[MUGGLE_LOG_MSG_MAX_LEN];
	snprintf(expect, sizeof(expect),
		"%s|%s:%u|%llu.%06d|%llu - %s\n",
		muggle_log_level_to_str(msg.level),
		"unittest_log.cpp", msg.src_loc.line,
		(unsigned long long)msg.ts.tv_sec,
		(int)msg.ts.tv_nsec / 1000,
		(unsigned long long)msg.tid,
		msg.payload);

	ASSERT_STREQ(buf, expect);
}
