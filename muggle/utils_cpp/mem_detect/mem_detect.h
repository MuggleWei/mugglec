/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_MEMORY_DETECT_H_
#define MUGGLE_MEMORY_DETECT_H_

#include "muggle/base_c/macro.h"
#include "muggle/utils_cpp/utils_cpp_macro.h"

#if MUGGLE_PLATFORM_WINDOWS

// detect memory leak
#if defined(_WIN32) && MUGGLE_DEBUG
#define _CRTDBG_MAP_ALLOC 
#include <crtdbg.h>
#endif

#endif

NS_MUGGLE_BEGIN

class MUGGLE_UTILS_CPP_EXPORT DebugMemoryLeakDetect
{
public:
	void Start();
	void End();

private:
#if MUGGLE_PLATFORM_WINDOWS && MUGGLE_DEBUG
	_CrtMemState start_;
	_CrtMemState end_;
#endif
};

NS_MUGGLE_END

#endif