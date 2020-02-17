/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

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
