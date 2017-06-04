#ifndef RUN_TEST_BASE_CLOG_H__
#define RUN_TEST_BASE_CLOG_H__

#include "muggle/base_c/base_c.h"

typedef struct CustomLog1_tag
{
	MUGGLE_LOG_HANDLE_BASE_STRUCT
	MuggleSpinLock spin_lock;
	MuggleFile file;
}CustomLog1;

#define CUSTOM_MSG_SIZE 1024
typedef struct LoopListUnit_tag
{
	struct LoopListUnit_tag *next;
	struct LoopListUnit_tag *prev;
	char buf[CUSTOM_MSG_SIZE+1];
}LoopListUnit;

typedef struct CustomLog2_tag
{
	MUGGLE_LOG_HANDLE_BASE_STRUCT
	MuggleThread th;
	LoopListUnit *loop_list;
	LoopListUnit *push;
	LoopListUnit *takeout;
	int is_finished;
}CustomLog2;

void exampleDefaultLog();
void exampleCustomLog();

int CustomLogInit_SpinLock_File(struct MuggleLogHandle_tag *log_handle);
int CustomLogFunc_SpinLock_File(struct MuggleLogHandle_tag *log_handle, struct MuggleLogAttributeInfo_tag *attr, const char *msg);
int CustomLogDestroy_SpinLock_File(struct MuggleLogHandle_tag *log_handle);

int CustomLogInit_LoopList_Console(struct MuggleLogHandle_tag *log_handle);
int CustomLogFunc_LoopList_Console(struct MuggleLogHandle_tag *log_handle, struct MuggleLogAttributeInfo_tag *attr, const char *msg);
int CustomLogDestroy_LoopList_Console(struct MuggleLogHandle_tag *log_handle);
THREAD_ROUTINE_RETURN CustomLog_LoopList_Console_Routine(void *args);

#endif