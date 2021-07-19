## log

### 概念
* fmt: 格式化器
* handler: 日志处理器, 有console, file, file_rotate, file_time_rot四种handler
* logger: 日志实例, 有sync, async两种logger  
一个logger拥有多个handler, 每个handler只能被加入一个logger中, 每个handler都需要指定一个fmt

### 例子

#### 基本使用
```
#include "muggle/c/muggle_c.h"

int main()
{
	// 简单初始化默认的logger, 使用同步日志, 并有两个handler
	// console handler指定日志过滤级别为info
	// file rotate handler指定日志过滤级别为trace
	muggle_log_simple_init(
		MUGGLE_LOG_LEVEL_INFO,
		MUGGLE_LOG_LEVEL_TRACE);

	MUGGLE_LOG_INFO("Welcome to muggle c log");

	const char *name = "world";
	MUGGLE_LOG_INFO("hello %s", name);

	return 0;
}
```

#### console handler
```
#include "muggle/c/muggle_c.h"

void init_log()
{
	static muggle_log_console_handler_t console_handler;
	muggle_log_console_handler_init(&console_handler, 1);
	muggle_log_handler_set_level(
		(muggle_log_handler_t*)&console_handler, MUGGLE_LOG_LEVEL_DEBUG);

	muggle_logger_t *logger = muggle_logger_default();
	logger->add_handler(logger, (muggle_log_handler_t*)&console_handler);
}

int main()
{
	init_log();

	MUGGLE_LOG_INFO("default logger with console handler");

	return 0;
}
```

#### file handler
```
#include "muggle/c/muggle_c.h"

void init_log()
{
	static muggle_log_file_handler_t file_handler;
	muggle_log_file_handler_init(&file_handler, "log/hello.log", "w");
	muggle_log_handler_set_level(
		(muggle_log_handler_t*)&file_handler, MUGGLE_LOG_LEVEL_DEBUG);

	muggle_logger_t *logger = muggle_logger_default();
	logger->add_handler(logger, (muggle_log_handler_t*)&file_handler);
}

int main()
{
	init_log();

	MUGGLE_LOG_INFO("default logger with file handler");

	return 0;
}
```

#### file rotate handler
```
#include "muggle/c/muggle_c.h"

void init_log()
{
	static muggle_log_file_rotate_handler_t file_rot_handler;
	muggle_log_file_rotate_handler_init(
		&file_rot_handler, "log/example_rot.log",
		128, 5);
	muggle_log_handler_set_level(
		(muggle_log_handler_t*)&file_rot_handler, MUGGLE_LOG_LEVEL_DEBUG);

	muggle_logger_t *logger = muggle_logger_default();
	logger->add_handler(logger, (muggle_log_handler_t*)&file_rot_handler);
}

int main()
{
	init_log();

	for (int i = 0; i < 128; i++)
	{
		MUGGLE_LOG_INFO("default logger with file rotate handler, #%d", i);
	}

	return 0;
}
```

#### file time rotate handler
```
#include "muggle/c/muggle_c.h"

void init_log()
{
	static muggle_log_file_time_rot_handler_t file_time_rot_handler;
	muggle_log_file_time_rot_handler_init(
		&file_time_rot_handler, "log/example_time_rot.log",
		MUGGLE_LOG_TIME_ROTATE_UNIT_HOUR, 2, false);
	muggle_log_handler_set_level(
		(muggle_log_handler_t*)&file_time_rot_handler, MUGGLE_LOG_LEVEL_DEBUG);

	muggle_logger_t *logger = muggle_logger_default();
	logger->add_handler(logger, (muggle_log_handler_t*)&file_time_rot_handler);
}

int main()
{
	init_log();

	MUGGLE_LOG_INFO("default logger with file time rotate handler");

	return 0;
}
```

#### 自定义输出格式
```
#include "muggle/c/muggle_c.h"

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

void init_log()
{
	static muggle_log_fmt_t formatter = {
		MUGGLE_LOG_FMT_ALL,
		customize_log_fmt_func
	};

	static muggle_log_file_time_rot_handler_t file_time_rot_handler;
	muggle_log_file_time_rot_handler_init(
		&file_time_rot_handler, "log/example_time_rot.log",
		MUGGLE_LOG_TIME_ROTATE_UNIT_HOUR, 2, false);
	muggle_log_handler_set_level(
		(muggle_log_handler_t*)&file_time_rot_handler, MUGGLE_LOG_LEVEL_DEBUG);
	muggle_log_handler_set_fmt((muggle_log_handler_t*)&file_time_rot_handler, &formatter);

	muggle_logger_t *logger = muggle_logger_default();
	logger->add_handler(logger, (muggle_log_handler_t*)&file_time_rot_handler);
}

int main()
{
	init_log();

	MUGGLE_LOG_INFO("default logger with file time rotate handler and use customize formatter");

	return 0;
}
```

#### async logger
```
#include "muggle/c/muggle_c.h"

muggle_logger_t* my_async_logger()
{
	static muggle_async_logger_t async_logger;
	return (muggle_logger_t*)&async_logger;
}

void init_log()
{
	muggle_logger_t *logger = my_async_logger();
	muggle_async_logger_init((muggle_async_logger_t*)logger, 4096);

	static muggle_log_console_handler_t console_handler;
	muggle_log_console_handler_init(&console_handler, 1);
	muggle_log_handler_set_level(
		(muggle_log_handler_t*)&console_handler, MUGGLE_LOG_LEVEL_DEBUG);

	logger->add_handler(logger, (muggle_log_handler_t*)&console_handler);
}

int main()
{
	init_log();

	MUGGLE_LOG(my_async_logger(), MUGGLE_LOG_LEVEL_INFO, "hello async logger");

	muggle_logger_t *logger = my_async_logger();
	logger->destroy(logger);

	return 0;
}
```

#### 自定义日志宏
```
#include "muggle/c/muggle_c.h"

muggle_logger_t* my_async_logger()
{
	static muggle_async_logger_t async_logger;
	return (muggle_logger_t*)&async_logger;
}

#define MY_LOG(level, format, ...) \
do \
{ \
	muggle_log_src_loc_t loc_arg##__LINE__ = { \
		__FILE__, __LINE__, __FUNCTION__ \
	}; \
	muggle_logger_t *logger = my_async_logger(); \
	logger->log(logger, level, &loc_arg##__LINE__, format, ##__VA_ARGS__); \
} while (0)

#define LOG_TRACE(format, ...) MY_LOG(MUGGLE_LOG_LEVEL_TRACE, format, ##__VA_ARGS__)
#define LOG_DEBUG(format, ...) MY_LOG(MUGGLE_LOG_LEVEL_DEBUG, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...) MY_LOG(MUGGLE_LOG_LEVEL_INFO, format, ##__VA_ARGS__)
#define LOG_WARNING(format, ...) MY_LOG(MUGGLE_LOG_LEVEL_WARNING, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) MY_LOG(MUGGLE_LOG_LEVEL_ERROR, format, ##__VA_ARGS__)
#define LOG_FATAL(format, ...) MY_LOG(MUGGLE_LOG_LEVEL_FATAL, format, ##__VA_ARGS__)

int my_log_fmt_func(const muggle_log_msg_t *msg, char *buf, size_t bufsize)
{
	const char *level = muggle_log_level_to_str(msg->level);

	char filename[MUGGLE_MAX_PATH];
	muggle_path_basename(msg->src_loc.file, filename, sizeof(filename));

	struct tm t;
	gmtime_r(&msg->ts.tv_sec, &t);

	const char *payload = "";
	if (msg->payload)
	{
		payload = msg->payload;
	}

	return (int)snprintf(buf, bufsize,
		"%s|%d-%02d-%02dT%02d:%02d:%02d.%03d|%s.%u|%llu - %s\n",
		level,
		(int)t.tm_year+1900, (int)t.tm_mon+1, (int)t.tm_mday,
		(int)t.tm_hour, (int)t.tm_min, (int)t.tm_sec,
		(int)msg->ts.tv_nsec / 1000000,
		filename, (unsigned int)msg->src_loc.line,
		(unsigned long long)msg->tid,
		payload);
}

void init_log()
{
	muggle_logger_t *logger = my_async_logger();
	muggle_async_logger_init((muggle_async_logger_t*)logger, 4096);

	// fmt
	static muggle_log_fmt_t formatter = {
		MUGGLE_LOG_FMT_ALL,
		my_log_fmt_func
	};

	// console handler
	static muggle_log_console_handler_t console_handler;
	muggle_log_console_handler_init(&console_handler, 1);
	muggle_log_handler_set_fmt((muggle_log_handler_t*)&console_handler, &formatter);
	muggle_log_handler_set_level((muggle_log_handler_t*)&console_handler,
		MUGGLE_LOG_LEVEL_WARNING);

	// file time rotate handler
	static muggle_log_file_time_rot_handler_t time_rot_handler;
	muggle_log_file_time_rot_handler_init(
		&time_rot_handler, "log/example_logger.log",
		MUGGLE_LOG_TIME_ROTATE_UNIT_DAY, 1, false);
	muggle_log_handler_set_fmt((muggle_log_handler_t*)&time_rot_handler, &formatter);
	muggle_log_handler_set_level((muggle_log_handler_t*)&time_rot_handler,
		MUGGLE_LOG_LEVEL_TRACE);

	// add handler
	logger->add_handler(logger, (muggle_log_handler_t*)&console_handler);
	logger->add_handler(logger, (muggle_log_handler_t*)&time_rot_handler);
}

int main()
{
	init_log();

	LOG_TRACE("my logger - trace");
	LOG_DEBUG("my logger - debug");
	LOG_INFO("my logger - info");
	LOG_WARNING("my logger - warning");
	LOG_ERROR("my logger - error");

	muggle_logger_t *logger = my_async_logger();
	logger->destroy(logger);

	return 0;
}
```
