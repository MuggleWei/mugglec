- [log](#log)
	- [概念](#概念)
	- [基础使用](#基础使用)
		- [简单的初始化](#简单的初始化)
		- [简单的初始化2](#简单的初始化2)
		- [Debug日志](#debug日志)
		- [Assert](#assert)
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
本节讲解mugglec日志模块的基本概念及其用法

## 概念
先让我们从最基本的概念开始, mugglec的日志模块中有三个主要的对象, 分别是
* fmt: 格式化器
* handler: 日志处理器, 有`console`, `file`, `file_rotate`, `file_time_rot`四种handler
* logger: 日志实例, 有`sync`, `async`两种logger  


一个`logger`拥有多个`handler`, 每个`handler`只能被加入一个`logger`中, 每个`handler`都需要指定一个`fmt`

## 基础使用

### 简单的初始化
mugglec的日志模块内置了两个简单的开箱即用的初始化函数, 分别是`muggle_log_simple_init`和`muggle_log_complicated_init`, 使用他们可以方便的快速初始化日志库  
我们以一个输出hello world的程序开始吧 [log_simple.c](./simple/log_simple.c)
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
除了`muggle_log_simple_init`之外, 还可以使用`muggle_log_complicated_init`, 它也一样的简单易用, 稍微修改一下前一个例子便可得到 [log_simple2.c](./simple2/log_simple2.c)
```
int main()
{
	muggle_log_complicated_init(LOG_LEVEL_INFO, LOG_LEVEL_DEBUG, "log/log_simple2.log");
	LOG_INFO("hello world");

	return 0;
}
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
虽然用户可以通过日志等级过滤要输出的日志, 但有时用户想要在开发阶段输出各种等级的日志并且完全不影响生产, 可以使用`DEBUG_LOG_{等级}`, 这样在编译release的时候, 对应的日志输出位置将在预编译阶段就被完全排除: [log_dbg_rel.c](./dbg_rel/log_dbg_rel.c)
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

### Assert
在开发阶段, 经常会使用到assert, 在mugglec的日志库中提供了两种断言, 分别是`MUGGLE_ASSERT`和`MUGGLE_ASSERT_MSG`, 其中`MUGGLE_ASSERT_MSG`在断言中断时, 会打印出用户的附加信息
```
int v = 0;
MUGGLE_ASSERT_MSG(v == 1, "v must equal 1");
```

## 配置handler
用户也可以自定义的日志初始化, 让我们从`handler`开始逐个说明

### console handler
`console handler`用于命令行输出: [log_console_handler.c](./console_handler/log_console_handler.c)  
自定义的日志初始化函数
```
void init_log()
{
	static muggle_log_console_handler_t console_handler;
	muggle_log_console_handler_init(&console_handler, 1);
	muggle_log_handler_set_level(
		(muggle_log_handler_t*)&console_handler, LOG_LEVEL_DEBUG);

	muggle_logger_t *logger = muggle_logger_default();
	logger->add_handler(logger, (muggle_log_handler_t*)&console_handler);
}
```
* 使用`muggle_log_console_handler_init`来初始化一个`console handler`, 第二个参数代表在命令行输出时是否要带上颜色
* 使用`muggle_log_handler_set_level`来设置本`handler`的日志过滤等级
* `muggle_logger_default`用于获取默认的`logger`
* `logger->add_handler`将`handler`加入`logger`中

接着用自定义日志初始化函数替换掉原本的初始化
```
int main()
{
	init_log();
	
	LOG_INFO("info message");
	LOG_WARNING("warning message");
	LOG_ERROR("error message");

	return 0;
}
```
可以看到如下输出  
> INFO|log_console_handler.c:18 - info message  
> <font color="yellow">WARNING|log_console_handler.c:19 - warning message</font>  
> <font color="red">ERROR|log_console_handler.c:20 - error message</font>

由于在初始化`console_handler`时选择了开启颜色(第二个参数为1), 所以warning及以上级别在命令行日志输出会带有颜色

### file handler
`file_handler`用于文件输出: [log_file_handler.c](./file_handler/log_file_handler.c)  
让我们修改一下`console handler`的日志初始化函数
```
void init_log()
{
	static muggle_log_file_handler_t file_handler;
	muggle_log_file_handler_init(&file_handler, "log/hello.log", "w");
	muggle_log_handler_set_level(
		(muggle_log_handler_t*)&file_handler, LOG_LEVEL_DEBUG);

	muggle_logger_t *logger = muggle_logger_default();
	logger->add_handler(logger, (muggle_log_handler_t*)&file_handler);
}
```
* `muggle_log_file_handler_init`初始化文件处理器, 第二个参数设置输出路径为`log/hello.log`, 第三个参数设置为`"w"`, 这样每次日志都是重头写; 如果想要每次追加写, 可将第三个参数设置为`"a"`

### file rotate handler
`file_rotate_handler`依然用于写文件, 它相对于`file_handler`的优点在于: 当文件到达一定大小时, 会新建一个日志文件, 并将老的日志文件改名为`{日志名称}.1`, 而旧的文件将以此类推由`{日志名称}.{n}`变为`{日志名}.{n+1}`, 并根据配置, 将最老的一个文件删除: [log_file_rotate_handler.c](./file_rotate_handler/log_file_rotate_handler.c)
```
#include "muggle/c/muggle_c.h"

void init_log()
{
	static muggle_log_file_rotate_handler_t file_rot_handler;
	muggle_log_file_rotate_handler_init(
		&file_rot_handler, "log/example_rot.log",
		16 * 1024, 5);
	muggle_log_handler_set_level(
		(muggle_log_handler_t*)&file_rot_handler, LOG_LEVEL_DEBUG);

	muggle_logger_t *logger = muggle_logger_default();
	logger->add_handler(logger, (muggle_log_handler_t*)&file_rot_handler);
}

int main()
{
	init_log();
	
	for (int i = 0; i < 4096; i++)
	{
		LOG_INFO("default logger with file rotate handler, #%d", i);
	}

	return 0;
}
```
* `muggle_log_file_rotate_handler_init`用于初始化`file_rotate_handler`, 其中
  * 第2个参数用于指定输出文件路径
  * 第3个参数用于指定单个日志文件最大的大小, 单位是字节, 这里设置为16KB
  * 最后一个参数指定最多保留几个旧日志

### file time rotate handler
如果想要根据时间来旋转日志文件而不是大小的话, 可以使用`file_time_rot`类型的handler: [log_file_time_rot_handler.c](./file_time_rot_handler/log_file_time_rot_handler.c)
```
void init_log()
{
	static muggle_log_file_time_rot_handler_t file_time_rot_handler;
	muggle_log_file_time_rot_handler_init(
		&file_time_rot_handler, "log/example_fmt.log",
		MUGGLE_LOG_TIME_ROTATE_UNIT_DAY, 1, false);
	muggle_log_handler_set_level(
		(muggle_log_handler_t*)&file_time_rot_handler, LOG_LEVEL_DEBUG);

	muggle_logger_t *logger = muggle_logger_default();
	logger->add_handler(logger, (muggle_log_handler_t*)&file_time_rot_handler);
}
```
* `muggle_log_file_time_rot_handler_init`用于初始化`time_rot_handler`, 其中
  * 第2个参数用于指定输出文件路径
  * 第3个参数用于指定文件旋转周期单位, 例子中MUGGLE_LOG_TIME_ROTATE_UNIT_DAY表示"天"
  * 第4个参数用于指定文件旋转周期的模, 例子中指定为1, 配合上周期单位表示每天都旋转
  * 最后一个旋转是否根据本地时区: true-根据本地时区旋转, false-根据UTC+0时区旋转

## 配置输出格式
用户还可以自定义`fmt`: [log_fmt.c](./fmt/log_fmt.c)
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
		MUGGLE_LOG_TIME_ROTATE_UNIT_DAY, 1, false);
	muggle_log_handler_set_level(
		(muggle_log_handler_t*)&file_time_rot_handler, LOG_LEVEL_DEBUG);
	muggle_log_handler_set_fmt((muggle_log_handler_t*)&file_time_rot_handler, &formatter);

	muggle_logger_t *logger = muggle_logger_default();
	logger->add_handler(logger, (muggle_log_handler_t*)&file_time_rot_handler);
}
```
* `customize_log_fmt_func`是用于格式化日志的函数
* `muggle_log_fmt_t formatter`自定义了一个日志`fmt`对象
  * 第1个参数是对`logger`的格式化提示, 比如`logger`中所有`handler`持有的`fmt`都不包含MUGGLE_LOG_FMT_TIME的情况时, 那么`logger`在准备日志信息时便不会去获取时间
  * 第2个参数是日志格式化函数
* `muggle_log_handler_set_fmt`为一个`handler`指定`fmt`

## 配置logger
前面的所有例子都使用了默认的`logger`, mugglec的日志模块还支持用户自定义`logger`

### async logger
默认的`logger`为同步模式, 也就是`sync`类型, 当想要日志异步输出, 可以使用`async`类型的`logger`: [log_async_logger.c](./async_logger/log_async_logger.c)
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
		(muggle_log_handler_t*)&console_handler, LOG_LEVEL_DEBUG);

	logger->add_handler(logger, (muggle_log_handler_t*)&console_handler);
}

void destroy_log()
{
	muggle_logger_t *logger = my_async_logger();
	logger->destroy(logger);
}

int main()
{
	init_log();

	MUGGLE_LOG(my_async_logger(), LOG_LEVEL_INFO, "hello async logger");

	destroy_log();

	return 0;
}
```
* 其中`muggle_async_logger_init`用于初始化一个异步日志, 第2个参数表示用于异步日志通信的`channel`的容量

## 自定义日志宏
到目前为止, 日志模块的基本功能已经全部介绍完毕了, 那么现在让我们把这些功能串起来使用. 定义一个自己的日志宏 [log_customize.c](./customize/log_customize.c)
* 异步`logger`
* 往`logger`中添加`console`和`file_time_rot`的`handler`
* 自定义`fmt`
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

#define MY_LOG_TRACE(format, ...) MY_LOG(MUGGLE_LOG_LEVEL_TRACE, format, ##__VA_ARGS__)
#define MY_LOG_DEBUG(format, ...) MY_LOG(MUGGLE_LOG_LEVEL_DEBUG, format, ##__VA_ARGS__)
#define MY_LOG_INFO(format, ...) MY_LOG(MUGGLE_LOG_LEVEL_INFO, format, ##__VA_ARGS__)
#define MY_LOG_WARNING(format, ...) MY_LOG(MUGGLE_LOG_LEVEL_WARNING, format, ##__VA_ARGS__)
#define MY_LOG_ERROR(format, ...) MY_LOG(MUGGLE_LOG_LEVEL_ERROR, format, ##__VA_ARGS__)
#define MY_LOG_FATAL(format, ...) MY_LOG(MUGGLE_LOG_LEVEL_FATAL, format, ##__VA_ARGS__)

int my_log_fmt_func(const muggle_log_msg_t *msg, char *buf, size_t bufsize)
{
	const char *level = muggle_log_level_to_str(msg->level);

	char filename[MUGGLE_MAX_PATH];
	muggle_path_basename(msg->src_loc.file, filename, sizeof(filename));

	struct tm t;
	localtime_r(&msg->ts.tv_sec, &t);

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
		&time_rot_handler, "log/log_customize.log",
		MUGGLE_LOG_TIME_ROTATE_UNIT_DAY, 1, true);
	muggle_log_handler_set_fmt((muggle_log_handler_t*)&time_rot_handler, &formatter);
	muggle_log_handler_set_level((muggle_log_handler_t*)&time_rot_handler,
		MUGGLE_LOG_LEVEL_TRACE);

	// add handler
	logger->add_handler(logger, (muggle_log_handler_t*)&console_handler);
	logger->add_handler(logger, (muggle_log_handler_t*)&time_rot_handler);
}

void destroy_log()
{
	muggle_logger_t *logger = my_async_logger();
	logger->destroy(logger);
}

int main()
{
	init_log();

	MY_LOG_TRACE("my logger - trace");
	MY_LOG_DEBUG("my logger - debug");
	MY_LOG_INFO("my logger - info");
	MY_LOG_WARNING("my logger - warning");
	MY_LOG_ERROR("my logger - error");

	destroy_log();

	return 0;
}
```
