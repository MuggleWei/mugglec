/******************************************************************************
 *  @file         muggle_benchmark_macro.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2022-01-02
 *  @copyright    Copyright 2022 Muggle Wei
 *  @license      MIT License
 *  @brief        benchmark macro
 *****************************************************************************/

#ifndef MUGGLE_C_BENCHMARK_MACRO_H_
#define MUGGLE_C_BENCHMARK_MACRO_H_

#include "muggle/c/muggle_c.h"

 // lib and dll
#if MUGGLE_PLATFORM_WINDOWS && defined(MUGGLE_BENCHMARK_USE_DLL)
	#ifdef MUGGLE_BENCHMARK_EXPORTS
		#define MUGGLE_BENCHMARK_EXPORT __declspec(dllexport)
	#else
		#define MUGGLE_BENCHMARK_EXPORT __declspec(dllimport)
	#endif
#else
	#define MUGGLE_BENCHMARK_EXPORT
#endif

#endif // !MUGGLE_C_BENCHMARK_MACRO_H_
