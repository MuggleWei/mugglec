#include "stacktrace.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <execinfo.h>
#include "muggle/c/log/log.h"

int muggle_stacktrace_get(muggle_stacktrace_t *st, unsigned int max_cnt_frame)
{
	// NOTE: copy stack info into muggle_stacktrace_t, avoid bug
#if MUGGLE_PLATFORM_WINDOWS
	return -1;
#else
	memset(st, 0, sizeof(muggle_stacktrace_t));

	void *stacks[MUGGLE_MAX_STACKTRACE_FRAME_NUM];
	char **symbols;

	if (max_cnt_frame == 0 || max_cnt_frame > MUGGLE_MAX_STACKTRACE_FRAME_NUM)
	{
		max_cnt_frame = MUGGLE_MAX_STACKTRACE_FRAME_NUM;
	}
	unsigned int cnt_frame = backtrace(stacks, max_cnt_frame);
	if (cnt_frame <= 1)
	{
		return -1;
	}

	symbols = backtrace_symbols(stacks, cnt_frame);
	if (symbols == NULL)
	{
		return -1;
	}

	st->symbols = malloc((cnt_frame - 1) * sizeof(char*));
	if (st->symbols == NULL)
	{
		MUGGLE_ERROR("failed allocate memory for stack strace symbols");
		free(symbols);
		return -1;
	}

	// NOTE: this start with 1
	for (unsigned int i = 1; i < cnt_frame; ++i)
	{
		unsigned int idx = i - 1;
		size_t len = strlen(symbols[i]);
		st->symbols[idx] = malloc(len + 1);
		if (st->symbols[idx] == NULL)
		{
			MUGGLE_ERROR("failed allocate memory for stack strace single symbol");
			muggle_stacktrace_free(st);
			free(symbols);
			return -1;
		}
		memcpy(st->symbols[idx], symbols[i], len);
		st->symbols[idx][len] = '\0';
	}
	st->cnt_frame = cnt_frame - 1;

	free(symbols);
	return 0;
#endif
}

void muggle_stacktrace_free(muggle_stacktrace_t *st)
{
#if MUGGLE_PLATFORM_WINDOWS
#else
	if (st->symbols)
	{
		for (unsigned int i = 0; i < st->cnt_frame; ++i)
		{
			if (st->symbols[i] == NULL)
			{
				break;
			}
			free(st->symbols[i]);
		}
		free(st->symbols);
	}
#endif
}

void muggle_print_stacktrace()
{
#if MUGGLE_PLATFORM_WINDOWS
#else
	void *stacks[MUGGLE_MAX_STACKTRACE_FRAME_NUM];
	unsigned int cnt_frame = backtrace(stacks, MUGGLE_MAX_STACKTRACE_FRAME_NUM);
	char **symbols = backtrace_symbols(stacks, cnt_frame);
	if (symbols == NULL)
	{
		return;
	}

	for (unsigned int i = 1; i < cnt_frame; ++i)
	{
		fprintf(stdout, "#%u %s\n", i - 1, symbols[i]);
	}

	free(symbols);
#endif
}
