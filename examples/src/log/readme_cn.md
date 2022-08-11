- [log](#log)
	- [概念](#概念)
	- [基础使用](#基础使用)
		- [简单的初始化](#简单的初始化)
		- [简单的初始化2](#简单的初始化2)
		- [Debug日志](#debug日志)
	- [配置handler](#配置handler)
		- [console handler](#console-handler)
		- [file handler](#file-handler)
		- [file rotate handler](#file-rotate-handler)
		- [file time rotate handler](#file-time-rotate-handler)
	- [配置输出格式](#配置输出格式)
	- [配置logger](#配置logger)
		- [async logger](#async-logger)
	- [自定义日志宏](#自定义日志宏)

# log
本节讲解mugglec日志模块的基本概念以及展示了其用法

## 概念
先让我们从最基本的概念开始, mugglec的日志模块中有三个主要的对象, 分别是
* fmt: 格式化器
* handler: 日志处理器, 有console, file, file_rotate, file_time_rot四种handler
* logger: 日志实例, 有sync, async两种logger  


一个logger拥有多个handler, 每个handler只能被加入一个logger中, 每个handler都需要指定一个fmt

## 基础使用

### 简单的初始化
mugglec的日志模块内置了两个简单的开箱即用的初始化函数, 分别是`muggle_log_simple_init`和`muggle_log_complicated_init`, 使用他们可以方便的快速初始化日志库  
我们以一个输出hello world的程序开始吧 [log_simple.c](./log_simple/log_simple.c)
```
#include "muggle/c/muggle_c.h"

int main()
{
    muggle_log_simple_init(MUGGLE_LOG_LEVEL_INFO, MUGGLE_LOG_LEVEL_DEBUG);
    MUGGLE_LOG_INFO("hello world");
    return 0;
}
```

运行此程序, 将在命令行显示类似于如下的输出, 并且在当前工作目录下生成文件`log/example_log_simple.log`, 其中有与命令行相同的输出. 格式为`{日志等级}|{时间(精确到纳秒)}|{日志所在的文件行}|{日志所在的函数}|{线程id} - {消息}`  
```
INFO|1660235969.000796146|log_simple.c:7|main|6140 - hello world
```

接着我们再来看看上面的代码
* 我们使用`muggle_log_simple_init`简单的初始化日志库, 第一个参数为命令行输出过滤等级, 第二个参数为文件输出的过滤等级. 当想关闭对应的输出时, 可以在对应的参数中填入`-1`即可
* 使用`MUGGLE_LOG_INFO`来输出INFO级别日志, 日志库默认有6个日志级别, 分别为TRACE, DEBUG, INFO, WARNING, ERROR, FATAL, 如果用户想自定义级别, 可在指定的级别上加上n(其中0 <= n <= 255)

除此之外还需要注意, `muggle_log_simple_init`的文件输出处理器使用了`file_rotate`, 每条日志都会追加到文件末尾. 当文件大小到64MB之后, 默认会创建新的文件, 而旧的文件将被改名为`${程序名称}.log.1`, 而原本更老的`${程序名称}.log.1`将会被改名为`${程序名称}.log.2`, 以此类推, 最多保留5个文件

### 简单的初始化2
除了`muggle_log_simple_init`之外, 还有一个一样简单的函数是`muggle_log_complicated_init`, 它也一样的简单易用, 稍微修改一下前一个例子便可得到 [log_simple2.c](./log_simple2/log_simple2.c)
```
muggle_log_complicated_init(LOG_LEVEL_INFO, LOG_LEVEL_DEBUG, "log/log_simple2.log");
LOG_INFO("hello world");
```
运行此程序, 将得到如下类似的日志输出, 并生成文件`log_simple2.log.{yyyymmdd}`, 其中`{yyyymmdd}`代表的是日志输出时刻的UTC+0时区的年月日
```
INFO|2022-08-11T16:40:13.105|log_simple2.c:7|main|14144 - hello world
```

* `muggle_log_complicated_init`比`muggle_log_simple_init`多了一个参数, 可以让用户指定输出的目录
* 日志输出中的的个信息, 换成了格式化的UTC+0时区的时间字符串
* 相比于`muggle_log_simple_init`的文件输出使用的`file_rotate`处理器, `muggle_log_complicated_init`使用的是`file_time_rot`, 在日志名称之后追加了年月日, 按照UTC+0时区每天自动创建一个新的日志文件

仔细观察还可以发现, 这次的日志输出使用的是`LOG_INFO`而非`MUGGLE_LOG_INFO`, 这是因为在例子的cmake文件中加入了`MUGGLE_HOLD_LOG_MACRO`宏定义, 从而可以直接使用`LOG_INFO`调用日志库的输出.  

### Debug日志
虽然用户可以通过日志等级过滤要输出的日志, 但有时用户想要在开发阶段输出各种等级的日志并在生产当中完全不显示, 可以使用`DEBUG_LOG_{等级}`, 这样在编译release的时候, 对应的日志输出位置将在预编译阶段就被完全排除[log_dbg_rel.c](./log_dbg_rel/log_dbg_rel.c)
```
muggle_log_complicated_init(LOG_LEVEL_INFO, LOG_LEVEL_DEBUG, "log/dbg_rel.log");

DEBUG_LOG_DEBUG("debug debug");
DEBUG_LOG_INFO("debug info");
DEBUG_LOG_WARNING("debug warning");

LOG_DEBUG("debug & release debug");
LOG_INFO("debug & release info");
LOG_WARNING("debug & release warning");
```
此时设置的命令行过滤等级为INFO, 文件过滤等级为DEBUG, 并且6条日志当中有3条是`DEBUG_LOG_{等级}`打印的, 因此
* 当编译Debug版本时, 命令行可以看到4条输出, 文件可以看到6条输出
* 当编译Release版本时, 命令行可以看到2条输出, 文件可以看到3条输出

## 配置handler

### console handler
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

### file handler
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

### file rotate handler
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

### file time rotate handler
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

## 配置输出格式
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

## 配置logger

### async logger
默认的logger为同步模式, 当想要日志异步输出, 可以使用async logger
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

## 自定义日志宏
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
