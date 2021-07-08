/******************************************************************************
 *  @file         version.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-22
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec version
 *****************************************************************************/

#include "version.h"
#include <stdio.h>

#ifndef MUGGLE_C_SEMVER
	#define MUGGLE_C_SEMVER ?.?.?
#endif
#define MUGGLE_C_COMPILRE_TIME __DATE__ " " __TIME__

const char* mugglec_version()
{
	return MUGGLE_MACRO_STR(MUGGLE_C_SEMVER);
}

const char* mugglec_compile_time()
{
	return MUGGLE_C_COMPILRE_TIME;
}
