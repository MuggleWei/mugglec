#include "example_log.h"


int main(int argc, char *argv[])
{
	char log_file[MUGGLE_MAX_PATH];

	// log without initialize
	MUGGLE_INFO("Muggle log Test\n");

	// initialize log
	MuggleGetAbsolutePath("muggleLog.txt", log_file);
	MuggleLogDefaultInit(log_file, 1);
	MuggleLogDefaultFlags(MUGGLE_LOG_DEFAULT_CONSOLE, 0);
	MuggleLogDefaultFlags(MUGGLE_LOG_DEFAULT_FILE, 0);
	MuggleLogDefaultFlags(MUGGLE_LOG_DEFAULT_WIN_DEBUG_OUT, 0);

	MUGGLE_WARNING("The efficiency of default log hangle is not high, just for write simple code conveniently\n");
	MUGGLE_WARNING("If you wanna use muggle log in real project, avoid use default category, unless you change the handles in default category\n");

	exampleDefaultLog();
	exampleCustomLog();

	MuggleLogDefaultClear();

	return 0;
}

void exampleDefaultLog()
{
	MUGGLE_INFO("\n");
	MUGGLE_INFO("Already initialize default log handle\n");
	MUGGLE_WARNING("If terminal support, u can see yellow in this line\n");
	MUGGLE_INFO("Now log text will output to console, file and vs debug output window if in vs\n");

	// change format
	MUGGLE_INFO("\n");
	MUGGLE_WARNING("You may be already notice except the first line, the text only with message\n");
	MUGGLE_INFO("You can change default output format with MuggleLogDefaultFlags like below\n");

	MuggleLogDefaultFlags(MUGGLE_LOG_DEFAULT_CONSOLE, MUGGLE_LOG_FMT_LEVEL);
	MUGGLE_INFO("hello world\n");
	MuggleLogDefaultFlags(MUGGLE_LOG_DEFAULT_CONSOLE, MUGGLE_LOG_FMT_FILE);
	MUGGLE_INFO("hello world\n");
	MuggleLogDefaultFlags(MUGGLE_LOG_DEFAULT_CONSOLE, MUGGLE_LOG_FMT_LINE);
	MUGGLE_INFO("hello world\n");
	MuggleLogDefaultFlags(MUGGLE_LOG_DEFAULT_CONSOLE, MUGGLE_LOG_FMT_FUNC);
	MUGGLE_INFO("hello world\n");
	MuggleLogDefaultFlags(MUGGLE_LOG_DEFAULT_CONSOLE, MUGGLE_LOG_FMT_TIME);
	MUGGLE_INFO("hello world\n");
	MuggleLogDefaultFlags(MUGGLE_LOG_DEFAULT_CONSOLE, 0);

	// close and open default log handle
	MUGGLE_INFO("\n");
	MuggleLogDefaultRemove(MUGGLE_LOG_DEFAULT_FILE);
	MuggleLogDefaultRemove(MUGGLE_LOG_DEFAULT_WIN_DEBUG_OUT);
	MUGGLE_INFO("I already close file output and windows debug output, now only output to console\n");

	// note the pit of file log handle
	MUGGLE_INFO("\n");
	MUGGLE_WARNING("NOTE: by default, I will open log file with trunc flag\n");
	MUGGLE_INFO("If it not u want, you can create your own MuggleLogHandle and use MUGGLE_LOG for output.\n");
}

void exampleCustomLog()
{
	// customize a log handle
	MUGGLE_INFO("\n");
	MUGGLE_WARNING("Create log handle for yourself\n");
	MUGGLE_WARNING("If use another log handle for console output, remember to close default console log or ensure "
		"not use default console log in the same time, otherwise it may lead confusing output in multiple thread\n");

	MuggleLogCategory *log_category = MuggleLogGenCategroy("root", 5);
	unsigned int priority = (5 << MUGGLE_LOG_LEVEL_OFFSET) + 15;
	int i;
	MuggleLogHandle *lh1, *lh2;

	lh1 = MuggleLogGenHandle(
		"hello", MUGGLE_LOG_FMT_FILE | MUGGLE_LOG_FMT_LINE,
		sizeof(CustomLog1), CustomLogFunc_SpinLock_File,
		MuggleLogGenFmtText, CustomLogInit_SpinLock_File, CustomLogDestroy_SpinLock_File);
	lh2 = MuggleLogGenHandle(
		"second", 0, sizeof(CustomLog2), CustomLogFunc_LoopList_Console,
		MuggleLogGenFmtText, CustomLogInit_LoopList_Console, CustomLogDestroy_LoopList_Console);

	MuggleLogCategoryAddHandle(log_category, lh1);
	MuggleLogCategoryAddHandle(log_category, lh2);
	MUGGLE_LOG(log_category, priority, "log with customize log function\n");
	for (i = 0; i < 20; ++i)
	{
		MUGGLE_LOG(log_category, (i << MUGGLE_LOG_LEVEL_OFFSET), "try to output log in level #%d\n", i);
	}

	MuggleLogClearCategory(log_category);
	free(log_category);
}


int CustomLogInit_SpinLock_File(struct MuggleLogHandle_tag *log_handle)
{
	CustomLog1 *p = (CustomLog1*)log_handle;
	char file_path[MUGGLE_MAX_PATH] = { 0 };
	int flags, attr;

	if (!MuggleGetAbsolutePath("customLog.txt", file_path))
	{
		MUGGLE_ERROR("Failed in get absolute path\n");
		return -1;
	}
	flags = MUGGLE_FILE_WRITE | MUGGLE_FILE_READ | MUGGLE_FILE_APPEND | MUGGLE_FILE_CREAT;
	attr = MUGGLE_PERM_USER_READ | MUGGLE_PERM_USER_WRITE | MUGGLE_PERM_GRP_READ;
	if (!MuggleFileOpen(&p->file, file_path, flags, attr))
	{
		MUGGLE_ERROR("Failed create file handle: %s\n", file_path);
		return -1;
	}

	if (!MuggleInitSpinLock(&p->spin_lock))
	{
		MUGGLE_ERROR("Failed init a mutex\n");
		return -1;
	}

	return 0;
}
int CustomLogFunc_SpinLock_File(struct MuggleLogHandle_tag *log_handle, struct MuggleLogAttributeInfo_tag *attr, const char *msg)
{
	char buf[1024] = { 0 };
	int write_num = MuggleLogGenFmtText(log_handle, attr, msg, buf, 1024);
	CustomLog1 *p = (CustomLog1*)log_handle;

	MuggleLockSpinLock(&p->spin_lock);
	MuggleFileWrite(&p->file, buf, (long)write_num);
	MuggleUnlockSpinLock(&p->spin_lock);

	return write_num;
}
int CustomLogDestroy_SpinLock_File(struct MuggleLogHandle_tag *log_handle)
{
	CustomLog1 *p = (CustomLog1*)log_handle;
	MuggleDestroySpinLock(&p->spin_lock);
	MuggleFileClose(&p->file);
	free(p);

	return 0;
}

int CustomLogInit_LoopList_Console(struct MuggleLogHandle_tag *log_handle)
{
	CustomLog2 *p = (CustomLog2*)log_handle;
	int i;

	p->is_finished = 0;
	p->loop_list = (LoopListUnit*)malloc(sizeof(LoopListUnit) * 16);
	for (i = 0; i < 15; ++i)
	{
		p->loop_list[i].next = &p->loop_list[i + 1];
		p->loop_list[i+1].prev = &p->loop_list[i];
	}
	p->loop_list[15].next = &p->loop_list[0];
	p->loop_list[0].prev = &p->loop_list[15];

	p->push = p->takeout = p->loop_list;
	MuggleThreadCreate(&p->th, NULL, CustomLog_LoopList_Console_Routine, (void*)p);

	return 0;
}
int CustomLogFunc_LoopList_Console(struct MuggleLogHandle_tag *log_handle, struct MuggleLogAttributeInfo_tag *attr, const char *msg)
{
	CustomLog2 *p = (CustomLog2*)log_handle;
	int num_write = 0;

	if (p->push->next != p->takeout)
	{
		num_write = MuggleLogGenFmtText(log_handle, attr, msg, p->push->buf, CUSTOM_MSG_SIZE);
		p->push = p->push->next;
	}

	return num_write;
}
int CustomLogDestroy_LoopList_Console(struct MuggleLogHandle_tag *log_handle)
{
	CustomLog2 *p = (CustomLog2*)log_handle;
	p->is_finished = 1;
	MSleep(30);

	MuggleThreadWaitExit(&p->th);
	free(p->loop_list);
	free(p);

	return 0;
}

THREAD_ROUTINE_RETURN CustomLog_LoopList_Console_Routine(void *args)
{
	CustomLog2 *p = (CustomLog2*)args;
	while (!p->is_finished || p->takeout != p->push)
	{
		if (p->takeout != p->push)
		{
			printf(p->takeout->buf);
			p->takeout = p->takeout->next;
		}
	}

	return 0;
}