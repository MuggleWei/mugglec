/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef __MUGGLE_CPP_UTILS_MACRO_H__
#define __MUGGLE_CPP_UTILS_MACRO_H__

#if MUGGLE_PLATFORM_WINDOWS && defined(MUGGLE_USE_DLLS)
	#ifdef MUGGLE_CPP_UTLS_EXPORTS
		#define MUGGLE_CPP_UTLS_EXPORT __declspec(dllexport)
	#else
		#define MUGGLE_CPP_UTLS_EXPORT __declspec(dllimport)
	#endif
#else
	#define MUGGLE_CPP_UTLS_EXPORT
#endif

#endif