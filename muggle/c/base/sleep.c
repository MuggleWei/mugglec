/******************************************************************************
 *  @file         sleep.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-15
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec sleep source file
 *****************************************************************************/

#include "sleep.h"
#include "muggle/c/base/err.h"

#if MUGGLE_PLATFORM_WINDOWS
	#include <windows.h>
#else
	#include <unistd.h>
#endif

int muggle_msleep(unsigned long ms)
{
#if MUGGLE_PLATFORM_WINDOWS
	Sleep(ms);
	return MUGGLE_OK;
#else
	return usleep(ms * 1000) == 0 ? MUGGLE_OK : MUGGLE_ERR_INTERRUPT;
#endif
}
