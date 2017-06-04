/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "muggle/utils_cpp/mem_detect/mem_detect.h"
#include "muggle/base_c/log.h"

NS_MUGGLE_BEGIN

#if MUGGLE_PLATFORM_WINDOWS

void DebugMemoryLeakDetect::Start()
{
#if MUGGLE_DEBUG
	_CrtMemCheckpoint(&start_);
#endif
}
void DebugMemoryLeakDetect::End()
{
#if MUGGLE_DEBUG
	_CrtMemState diff;
	_CrtMemCheckpoint(&end_);
	if (_CrtMemDifference(&diff, &start_, &end_))
	{
		_CrtMemDumpStatistics(&diff);
		MUGGLE_ASSERT_MSG(0, "memory leak\n");
	}
	_CrtDumpMemoryLeaks();
#endif
}

#else

void DebugMemoryLeakDetect::Start()
{
}
void DebugMemoryLeakDetect::End()
{
}

#endif

NS_MUGGLE_END