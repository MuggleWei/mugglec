/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_UTILS_C_MACRO_H_
#define MUGGLE_UTILS_C_MACRO_H_

#include "muggle/base_c/macro.h"

#if MUGGLE_PLATFORM_WINDOWS && defined(MUGGLE_USE_DLLS)
	#ifdef MUGGLE_UTILS_C_EXPORTS
		#define MUGGLE_UTILS_C_EXPORT __declspec(dllexport)
	#else
		#define MUGGLE_UTILS_C_EXPORT __declspec(dllimport)
	#endif
#else
	#define MUGGLE_BASE_CPP_EXPORT
#endif

#endif