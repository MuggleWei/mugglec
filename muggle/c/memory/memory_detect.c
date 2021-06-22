/******************************************************************************
 *  @file         memory_detect.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-17
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec memory detect
 *****************************************************************************/
 
#include "memory_detect.h"
#include <assert.h>

#if MUGGLE_DEBUG & MUGGLE_PLATFORM_WINDOWS

void muggle_debug_memory_leak_start(struct muggle_debug_memory_state *state)
{
	_CrtMemCheckpoint(&state->start);
}

void muggle_debug_memory_leak_end(struct muggle_debug_memory_state *state)
{
	_CrtMemState diff;
	_CrtMemCheckpoint(&state->end);
	if (_CrtMemDifference(&diff, &state->start, &state->end))
	{
		_CrtMemDumpStatistics(&diff);
		// MUGGLE_ASSERT_MSG(0, "memory leak\n");
		assert(0);
	}
	_CrtDumpMemoryLeaks();
}

#endif
