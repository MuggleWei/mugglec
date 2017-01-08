/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef __MUGGLE_BASE_CPP_MACRO_H__
#define __MUGGLE_BASE_CPP_MACRO_H__

#include "muggle/base/macro.h"

#if MUGGLE_PLATFORM_WINDOWS && defined(MUGGLE_USE_DLLS)
	#ifdef MUGGLE_BASE_CPP_EXPORTS
		#define MUGGLE_BASE_CPP_EXPORT __declspec(dllexport)
	#else
		#define MUGGLE_BASE_CPP_EXPORT __declspec(dllimport)
	#endif
#else
	#define MUGGLE_BASE_CPP_EXPORT
#endif

#endif