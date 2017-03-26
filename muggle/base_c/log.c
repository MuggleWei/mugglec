/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "muggle/base_c/log.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#if MUGGLE_PLATFORM_WINDOWS
	#include <windows.h>
#else
	#include <unistd.h>
#endif
#include "muggle/base_c/file.h"
#include "muggle/base_c/lock.h"

 // terminal color for *nix
#define UNIX_TERMINAL_COLOR_NRM  "\x1B[0m"
#define UNIX_TERMINAL_COLOR_RED  "\x1B[31m"
#define UNIX_TERMINAL_COLOR_GRN  "\x1B[32m"
#define UNIX_TERMINAL_COLOR_YEL  "\x1B[33m"
#define UNIX_TERMINAL_COLOR_BLU  "\x1B[34m"
#define UNIX_TERMINAL_COLOR_MAG  "\x1B[35m"
#define UNIX_TERMINAL_COLOR_CYN  "\x1B[36m"
#define UNIX_TERMINAL_COLOR_WHT  "\x1B[37m"

#define MUGGLE_LOG_DEFAULT_FMT_FLAGS (MUGGLE_LOG_FMT_LEVEL | MUGGLE_LOG_FMT_FILE | MUGGLE_LOG_FMT_LINE)

 // default log priority string
const char* g_muggle_log_level_str[MUGGLE_LOG_LEVEL_MAX] = {
	"",
	"Info",
	"Warning",
	"Error"
};

// default log function
int MuggleLogDefaultConsole(MuggleLogHandle *log_handle, MuggleLogAttributeInfo *attr, const char *msg);
int MuggleLogDefaultFile(MuggleLogHandle *log_handle, MuggleLogAttributeInfo *attr, const char *msg);
int MuggleLogDefaultWinDebugOut(MuggleLogHandle *log_handle, MuggleLogAttributeInfo *attr, const char *msg);

int MuggleLogDefaultDestroy(MuggleLogHandle *log_handle);

// default log handle struct
typedef struct MuggleLogHandleDefault_tag
{
	struct MuggleLogHandle_tag *next;
	unsigned int flags;
	MugglePtrLogFunc log_func;
	MugglePtrLogFormatFunc format_func;
	MugglePtrLogDestroyFunc destroy_func;
	char name[MUGGLE_LOG_MAX_NAME_LEN + 1];
	MuggleMutexLock *mtx;
	MuggleFile *file;
}MuggleLogDefaultHandle;

int g_enable_console_color = 0;
const int g_default_fmt_flags = MUGGLE_LOG_FMT_LEVEL | MUGGLE_LOG_FMT_FILE | MUGGLE_LOG_FMT_LINE;
MuggleFile g_log_default_file = { 0 };
MuggleMutexLock g_log_default_mtx[MUGGLE_LOG_DEFAULT_MAX] = { 0 };

// default log handles
MuggleLogDefaultHandle g_log_default_handles[MUGGLE_LOG_DEFAULT_MAX] = {
	{ NULL, MUGGLE_LOG_DEFAULT_FMT_FLAGS, MuggleLogDefaultConsole, MuggleLogGenFmtText, MuggleLogDefaultDestroy, {"MuggleDefaultLogConsole"}, NULL, NULL },
	{ NULL, MUGGLE_LOG_DEFAULT_FMT_FLAGS, MuggleLogDefaultFile, MuggleLogGenFmtText, MuggleLogDefaultDestroy, {"MuggleDefaultLogFile"}, NULL, NULL },
	{ NULL, MUGGLE_LOG_DEFAULT_FMT_FLAGS, MuggleLogDefaultWinDebugOut, MuggleLogGenFmtText, MuggleLogDefaultDestroy, {"MuggleDefaultLogWinOutput"}, NULL, NULL },
};

// default log category
MuggleLogCategory g_log_default_category = {
	"MuggleLogDefaultCategory",
	0,
	(MuggleLogHandle*)&g_log_default_handles[MUGGLE_LOG_DEFAULT_CONSOLE]
};

MuggleLogHandle* MuggleLogGenHandle(
	const char *name,
	unsigned int flags,
	size_t size,
	MugglePtrLogFunc log_func,
	MugglePtrLogFormatFunc format_func,
	MugglePtrLogInitFunc init_func,
	MugglePtrLogDestroyFunc destroy_func) 
{
	size_t len;
	MuggleLogHandle *log_handle;

	size = size > sizeof(MuggleLogHandle) ? size : sizeof(MuggleLogHandle);
	log_handle = (MuggleLogHandle*)malloc(size);
	if (log_handle == NULL)
	{
		return NULL;
	}

	memset(log_handle, 0, size);
	log_handle->next = NULL;
	log_handle->flags = flags;
	log_handle->log_func = log_func;
	log_handle->format_func = format_func;
	log_handle->destroy_func = destroy_func;
	if (name != NULL)
	{
		len = strlen(name);
		if (len > 0)
		{
			len = len > MUGGLE_LOG_MAX_NAME_LEN ? MUGGLE_LOG_MAX_NAME_LEN : len;
			memcpy(log_handle->name, name, len);
		}
	}

	if (init_func != NULL)
	{
		if (init_func(log_handle) < 0)
		{
			free(log_handle);
			return NULL;
		}
	}

	return log_handle;
}

int MuggleLogDestroyHandle(MuggleLogHandle *log_handle)
{
	if (log_handle == NULL)
	{
		return MUGGLE_LOG_ERROR_CODE_NULL_HANDLE;
	}

	if (log_handle->destroy_func != NULL)
	{
		return log_handle->destroy_func(log_handle);
	}

	return 0;
}

MuggleLogCategory* MuggleLogGenCategroy(const char *name, unsigned int priority)
{
	MuggleLogCategory *category = (MuggleLogCategory*)malloc(sizeof(MuggleLogCategory));
	if (category == NULL)
	{
		return NULL;
	}

	memset(category, 0, sizeof(MuggleLogCategory));
	if (name != NULL)
	{
		size_t len = strlen(name);
		len = len > MUGGLE_LOG_MAX_NAME_LEN ? MUGGLE_LOG_MAX_NAME_LEN : len;
		memcpy(category->name, name, len);
	}
	category->priority = priority;

	return category;
}

int MuggleLogClearCategory(MuggleLogCategory *category)
{
	if (category == NULL)
	{
		return MUGGLE_LOG_ERROR_CODE_NULL_CATEGORY;
	}

	MuggleLogHandle *p = category->head, *next = NULL;
	while (p != NULL)
	{
		next = p->next;
		MuggleLogDestroyHandle(p);
		p = next;
	}

	return 0;
}

int MuggleLogCategoryAddHandle(MuggleLogCategory *category, MuggleLogHandle *log_handle)
{
	if (category == NULL)
	{
		return MUGGLE_LOG_ERROR_CODE_NULL_CATEGORY;
	}
	if (log_handle == NULL)
	{
		return MUGGLE_LOG_ERROR_CODE_NULL_HANDLE;
	}
	if (log_handle->next != NULL)
	{
		return MUGGLE_LOG_ERROR_CODE_HANDLE_REPEAT_MOUNT;
	}

	MuggleLogHandle *next = category->head;
	category->head = log_handle;
	log_handle->next = next;

	return 0;
}

int MuggleLogCategoryRemoveHandle(MuggleLogCategory *category, MuggleLogHandle *log_handle)
{
	if (category == NULL)
	{
		return MUGGLE_LOG_ERROR_CODE_NULL_CATEGORY;
	}
	if (log_handle == NULL)
	{
		return MUGGLE_LOG_ERROR_CODE_NULL_HANDLE;
	}

	MuggleLogHandle *p = category->head, *prev = NULL;
	if (p == log_handle)
	{
		category->head = p->next;
		return MuggleLogDestroyHandle(p);
	}
	else
	{
		prev = p;
		p = p->next;
		while (p != NULL)
		{
			if (p == log_handle)
			{
				prev->next = p->next;
				return MuggleLogDestroyHandle(p);
			}
		}
	}
	
	return MUGGLE_LOG_ERROR_CODE_COULDNOT_FOUND;
}

void MuggleLogFunction(MuggleLogCategory *category, MuggleLogAttributeInfo *attr, const char *format, ...)
{
	char msg[MUGGLE_LOG_MAX_LEN];
	va_list args;

	if (category == NULL)
	{
		return;
	}
	if (attr == NULL)
	{
		return;
	}

	if ((attr->level >> MUGGLE_LOG_LEVEL_OFFSET) < category->priority)
	{
		return;
	}

	va_start(args, format);
	vsnprintf(msg, MUGGLE_LOG_MAX_LEN, format, args);
	va_end(args);

	MuggleLogHandle *log_handle = category->head;
	while (log_handle != NULL)
	{
		log_handle->log_func(log_handle, attr, msg);
		log_handle = log_handle->next;
	}
}

int MuggleLogGenFmtText(MuggleLogHandle *log_handle, MuggleLogAttributeInfo *attr, const char *msg, char *buf, int max_len)
{
	int remaining = max_len, num_write = 0;
	char *p = buf;

	if (buf == NULL || max_len <= 0)
	{
		return 0;
	}

	if (log_handle->flags & MUGGLE_LOG_FMT_LEVEL)
	{
		int level = attr->level >> MUGGLE_LOG_LEVEL_OFFSET;
		if (level > 0 && level < MUGGLE_LOG_LEVEL_MAX)
		{
			num_write = snprintf(p, remaining, "<L>%s|", g_muggle_log_level_str[level]);
			if (num_write == -1)
			{
				return max_len;
			}
			remaining -= num_write;
			p += num_write;
		}		
	}
	if (log_handle->flags & MUGGLE_LOG_FMT_FILE)
	{
		num_write = snprintf(p, remaining, "<F>%s|", attr->file);
		if (num_write == -1)
		{
			return max_len;
		}
		remaining -= num_write;
		p += num_write;
	}
	if (log_handle->flags & MUGGLE_LOG_FMT_LINE)
	{
		num_write = snprintf(p, remaining, "<l>%d|", attr->line);
		if (num_write == -1)
		{
			return max_len;
		}
		remaining -= num_write;
		p += num_write;
	}
	if (log_handle->flags & MUGGLE_LOG_FMT_FUNC)
	{
		num_write = snprintf(p, remaining, "<f>%s|", attr->func);
		if (num_write == -1)
		{
			return max_len;
		}
		remaining -= num_write;
		p += num_write;
	}
	if (log_handle->flags & MUGGLE_LOG_FMT_TIME)
	{
		num_write = snprintf(p, remaining, "<T>%ld|", (long)time(NULL));
		if (num_write == -1)
		{
			return max_len;
		}
		remaining -= num_write;
		p += num_write;
	}
	if (msg != NULL)
	{
		num_write = snprintf(p, remaining, "%s", msg);
		if (num_write == -1)
		{
			return max_len;
		}
		remaining -= num_write;
		p += num_write;
	}

	return max_len - remaining;
}

void MuggleLogDefaultInit(const char *log_file_path, int enable_console_color)
{
	int i;

	MuggleLogDefaultClear();

	g_enable_console_color = enable_console_color;	
	
	for (i = 0; i < MUGGLE_LOG_DEFAULT_MAX; ++i)
	{
		MuggleLogDefaultAdd(i, log_file_path);
	}
}
void MuggleLogDefaultClear()
{
	MuggleLogClearCategory(&g_log_default_category);
}
int MuggleLogDefaultAdd(int default_log_idx, const char *log_file_path)
{
	MuggleLogHandle *p = g_log_default_category.head;
	while (p != NULL)
	{
		if (p == (MuggleLogHandle*)&g_log_default_handles[default_log_idx])
		{
			return MUGGLE_LOG_ERROR_CODE_HANDLE_REPEAT_MOUNT;
		}
		p = p->next;
	}

	if (default_log_idx == MUGGLE_LOG_DEFAULT_FILE)
	{
		int flags = MUGGLE_FILE_WRITE | MUGGLE_FILE_READ | MUGGLE_FILE_APPEND | MUGGLE_FILE_CREAT | MUGGLE_FILE_TRUNC;
		int attr = MUGGLE_PERM_USER_READ | MUGGLE_PERM_USER_WRITE | MUGGLE_PERM_GRP_READ;

		if (log_file_path == NULL)
		{
			char log_file[MUGGLE_MAX_PATH];
			MuggleGetAbsolutePath("log.txt", log_file);
			MuggleFileOpen(&g_log_default_file, log_file, flags, attr);
		}
		else
		{
			MuggleFileOpen(&g_log_default_file, log_file_path, flags, attr);
		}
		g_log_default_handles[default_log_idx].file = (void*)&g_log_default_file;
	}	

	MuggleInitMutexLock(&g_log_default_mtx[default_log_idx]);
	g_log_default_handles[default_log_idx].mtx = &g_log_default_mtx[default_log_idx];
	MuggleLogCategoryAddHandle(&g_log_default_category, (MuggleLogHandle*)&g_log_default_handles[default_log_idx]);

	return 0;
}
void MuggleLogDefaultRemove(int default_log_idx)
{
	if (default_log_idx < 0 || default_log_idx >= MUGGLE_LOG_DEFAULT_MAX)
	{
		return;
	}
	MuggleLogCategoryRemoveHandle(&g_log_default_category, (MuggleLogHandle*)&g_log_default_handles[default_log_idx]);
}
void MuggleLogDefaultFlags(int default_log_idx, unsigned int flags)
{
	if (default_log_idx < 0 || default_log_idx >= MUGGLE_LOG_DEFAULT_MAX)
	{
		return;
	}
	g_log_default_handles[default_log_idx].flags = flags;
}
void MuggleLogDefaultAddHandle(MuggleLogHandle *log_handle)
{
	MuggleLogCategoryAddHandle(&g_log_default_category, log_handle);
}
void MuggleLogDefaultRemoveHandle(MuggleLogHandle *log_handle)
{
	MuggleLogCategoryRemoveHandle(&g_log_default_category, log_handle);
}

int MuggleLogDefaultConsole(MuggleLogHandle *log_handle, MuggleLogAttributeInfo *attr, const char *msg)
{
	char buf[MUGGLE_LOG_MAX_LEN + 1] = { 0 };
	FILE *fp = stdout;
	int need_color = 0;
	MuggleLogDefaultHandle *default_log_handle = (MuggleLogDefaultHandle*)log_handle;

	int write_num = MuggleLogGenFmtText(log_handle, attr, msg, buf, MUGGLE_LOG_MAX_LEN);

	if (default_log_handle->mtx != NULL)
	{
		MuggleLockMutexLock(default_log_handle->mtx);
	}

	if (attr->level >= MUGGLE_LOG_LEVEL_WARNING)
	{
		fp = stderr;
		need_color = 1;
	}

	if (g_enable_console_color && need_color)
	{
#if MUGGLE_PLATFORM_WINDOWS
		const HANDLE stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);

		// get the current text color
		CONSOLE_SCREEN_BUFFER_INFO sb_info;
		GetConsoleScreenBufferInfo(stdout_handle, &sb_info);
		const WORD old_sb_attrs = sb_info.wAttributes;

		// change text color
		fflush(stdout);
		if (attr->level >= MUGGLE_LOG_LEVEL_ERROR)
		{
			SetConsoleTextAttribute(stdout_handle, FOREGROUND_RED | FOREGROUND_INTENSITY);
		}
		else if (attr->level >= MUGGLE_LOG_LEVEL_WARNING)
		{
			SetConsoleTextAttribute(stdout_handle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
		}
		
		fprintf(fp, buf);

		fflush(stdout);

		// restores text color
		SetConsoleTextAttribute(stdout_handle, old_sb_attrs);
#else
		const char *color_str = UNIX_TERMINAL_COLOR_YEL;
		if (attr->level >= MUGGLE_LOG_LEVEL_ERROR)
		{
			color_str = UNIX_TERMINAL_COLOR_RED;
		}
		fprintf(fp, "%s%s", color_str, buf);
		fprintf(fp, "\033[m");
#endif
	}
	else
	{
		fprintf(fp, "%s", buf);
	}	

	if (default_log_handle->mtx != NULL)
	{
		MuggleUnlockMutexLock(default_log_handle->mtx);
	}

	return 0;
}
int MuggleLogDefaultFile(MuggleLogHandle *log_handle, MuggleLogAttributeInfo *attr, const char *msg)
{
	char buf[MUGGLE_LOG_MAX_LEN] = { 0 };
	int write_num = MuggleLogGenFmtText(log_handle, attr, msg, buf, MUGGLE_LOG_MAX_LEN);
	MuggleLogDefaultHandle *default_log_handle = (MuggleLogDefaultHandle*)log_handle;

	if (default_log_handle->mtx != NULL)
	{
		MuggleLockMutexLock(default_log_handle->mtx);
	}

	MuggleFile *fh = default_log_handle->file;
	if (fh != NULL)
	{
		MuggleFileWrite(fh, buf, (long)write_num);
	}	

	if (default_log_handle->mtx != NULL)
	{
		MuggleUnlockMutexLock(default_log_handle->mtx);
	}

	return 0;
}
int MuggleLogDefaultWinDebugOut(MuggleLogHandle *log_handle, MuggleLogAttributeInfo *attr, const char *msg)
{
#if MUGGLE_PLATFORM_WINDOWS
	char buf[MUGGLE_LOG_MAX_LEN] = { 0 };
	int write_num = MuggleLogGenFmtText(log_handle, attr, msg, buf, MUGGLE_LOG_MAX_LEN);
	MuggleLogDefaultHandle *default_log_handle = (MuggleLogDefaultHandle*)log_handle;

	if (default_log_handle->mtx != NULL)
	{
		MuggleLockMutexLock(default_log_handle->mtx);
	}

	WCHAR w_buf[MUGGLE_LOG_MAX_LEN];
	MultiByteToWideChar(CP_UTF8, 0, buf, -1, w_buf, MUGGLE_LOG_MAX_LEN);
	OutputDebugStringW(w_buf);

	if (default_log_handle->mtx != NULL)
	{
		MuggleUnlockMutexLock(default_log_handle->mtx);
	}
#endif
	return 0;
}

int MuggleLogDefaultDestroy(MuggleLogHandle *log_handle)
{
	MuggleLogDefaultHandle *default_log_handle = (MuggleLogDefaultHandle*)log_handle;
	if (default_log_handle->file != NULL)
	{
		MuggleFileClose(default_log_handle->file);
	}
	if (default_log_handle->mtx != NULL)
	{
		MuggleDestroyMutexLock(default_log_handle->mtx);
	}

	return 0;
}