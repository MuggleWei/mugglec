#include "example_log.h"
#include "muggle/base/base.h"

int main(int argc, char *argv[])
{
	char log_file[MUGGLE_MAX_PATH];

	// log without initialize
	MUGGLE_INFO("Muggle log Test\n");

	// initialize log
	FileGetAbsolutePath("muggleLog.txt", log_file);
	LogDefaultInit(log_file, 1);

	exampleLog();

	return 0;
}

void CustomLogFunc(struct LogHandle_tag *log_handle,struct LogAttribute_tag *attr,const char *msg)
{
	char buf[1024] = { 0 };
	int write_num = LogGenFmtText(log_handle, attr, msg, buf, 1024);

	if (log_handle->mtx != NULL)
	{
		MutexLock(log_handle->mtx);
	}

	FileHandle *fh = (FileHandle*)log_handle->io_target;
	if (fh != NULL)
	{
		FileHandleWrite(fh, buf, (long)write_num);
	}

	if (log_handle->mtx != NULL)
	{
		MutexUnLock(log_handle->mtx);
	}
}

void exampleLog()
{
	LogHandle lh = { 0 };
	FileHandle fh;
	MutexHandle mtx;
	char file_path[MUGGLE_MAX_PATH] = { 0 };
	int flags, attr;

	LogDefaultSwitch(MUGGLE_LOG_DEFAULT_CONSOLE, NULL, 1, 0);
	MUGGLE_INFO("\n");
	MUGGLE_INFO("Already initialize default log handle\n");
	MUGGLE_WARNING("If terminal support, u can see yellow in this line\n");
	MUGGLE_INFO("Now log text will output to console, file and vs debug output window if in vs\n");

	// use LogDefaultSwitch to change format
	MUGGLE_INFO("\n");
	MUGGLE_WARNING("You may be already notice after LogDefaultInit, the text only with message\n");
	MUGGLE_INFO("You can change default output format with LogDefaultSwitch like below\n");

	LogDefaultSwitch(MUGGLE_LOG_DEFAULT_CONSOLE, NULL, 1, MUGGLE_LOG_FMT_LEVEL);
	MUGGLE_INFO("hello world\n");
	LogDefaultSwitch(MUGGLE_LOG_DEFAULT_CONSOLE, NULL, 1, MUGGLE_LOG_FMT_FILE);
	MUGGLE_INFO("hello world\n");
	LogDefaultSwitch(MUGGLE_LOG_DEFAULT_CONSOLE, NULL, 1, MUGGLE_LOG_FMT_LINE);
	MUGGLE_INFO("hello world\n");
	LogDefaultSwitch(MUGGLE_LOG_DEFAULT_CONSOLE, NULL, 1, MUGGLE_LOG_FMT_FUNC);
	MUGGLE_INFO("hello world\n");
	LogDefaultSwitch(MUGGLE_LOG_DEFAULT_CONSOLE, NULL, 1, MUGGLE_LOG_FMT_TIME);
	MUGGLE_INFO("hello world\n");
	LogDefaultSwitch(MUGGLE_LOG_DEFAULT_CONSOLE, NULL, 1, 0);

	// use LogDefaultSwitch to close default log handle
	MUGGLE_INFO("\n");
	LogDefaultSwitch(MUGGLE_LOG_DEFAULT_CONSOLE, NULL, 1, 0);
	MUGGLE_WARNING("Except change format, LogDefaultSwitch also can open or close default log handle output\n");
	LogDefaultSwitch(MUGGLE_LOG_DEFAULT_FILE, NULL, 0, 0);
	LogDefaultSwitch(MUGGLE_LOG_DEFAULT_WIN_DEBUG_OUT, NULL, 0, 0);
	MUGGLE_INFO("I already close file and windows debug output log, now only output to console\n");

	// note the pit of file log handle
	MUGGLE_INFO("\n");
	MUGGLE_WARNING("NOTE: by default, I will open log file with trunc flag\n");
	MUGGLE_INFO("If it not u want, there two way to change\n");
	MUGGLE_INFO("1. create your own LogHandle and use MUGGLE_LOG for output\n");
	MUGGLE_INFO("2. create your own LogHandle and call LogDefaultSwitch to replace default log file output\n");

	// customize a log handle
	MUGGLE_INFO("\n");
	MUGGLE_WARNING("Create log handle for yourself\n");
	MUGGLE_INFO("Don't use another log handle for console output, it may lead confusing output in multiple thread\n");

	if (!FileGetAbsolutePath("customLog.txt", file_path))
	{
		MUGGLE_ERROR("Failed in get absolute path\n");
	}
	flags = MUGGLE_FILE_WRITE | MUGGLE_FILE_READ | MUGGLE_FILE_APPEND | MUGGLE_FILE_CREAT;
	attr = MUGGLE_FILE_ATTR_USER_READ | MUGGLE_FILE_ATTR_USER_WRITE | MUGGLE_FILE_ATTR_GRP_READ;
	if (!FileHandleOpen(&fh, file_path, flags, attr))
	{
		MUGGLE_ERROR("Failed create file handle: %s\n", file_path);
	}

	if (!MutexInit(&mtx))
	{
		MUGGLE_ERROR("Failed init a mutex\n");
	}

	lh.io_target = (void*)&fh;
	lh.func = CustomLogFunc;
	lh.mtx = &mtx;
	lh.format = 0;

	MUGGLE_LOG(&lh, MUGGLE_LOG_LEVEL_INFO, "log with customize log function\n");
	MUGGLE_INFO("Already write a log message into customLog.txt with MUGGLE_LOG\n");

	LogDefaultSwitch(MUGGLE_LOG_DEFAULT_FILE, &lh, 0, 0);
	MUGGLE_INFO("Replace default file log handle\n");

	LogDefaultSwitch(MUGGLE_LOG_DEFAULT_FILE, NULL, 1, 0);
	MUGGLE_INFO("Restore default file log handle\n");

	MutexDestroy(&mtx);
	FileHandleClose(&fh);
}