/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_MEMORY_DETECT_H_
#define MUGGLE_C_MEMORY_DETECT_H_

#include "muggle/c/base/macro.h"

EXTERN_C_BEGIN

#if MUGGLE_DEBUG & MUGGLE_PLATFORM_WINDOWS

// detect memory leak
#if defined(_WIN32) && MUGGLE_DEBUG
#define _CRTDBG_MAP_ALLOC 
#include <crtdbg.h>
#endif

struct muggle_debug_memory_state
{
	_CrtMemState start;
	_CrtMemState end;
};

MUGGLE_CC_EXPORT
void muggle_debug_memory_leak_start(struct muggle_debug_memory_state *state);

MUGGLE_CC_EXPORT
void muggle_debug_memory_leak_end(struct muggle_debug_memory_state *state);

#else

struct muggle_debug_memory_state
{
	void *v;
};

#define muggle_debug_memory_leak_start(state)
#define muggle_debug_memory_leak_end(state)

#endif

EXTERN_C_END

#endif
