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
#include <string.h>

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

const char* mugglec_compile_dt()
{
	static char dt[21] = {
		// year
		__DATE__[7], __DATE__[8], __DATE__[9], __DATE__[10], '-',
		// first month letter, Oct Nov Dec = '1' otherwise '0'
		(__DATE__[0] == 'O' || __DATE__[0] == 'N' || __DATE__[0] == 'D') ? '1' : '0',
		// Second month letter
		(__DATE__[0] == 'J') ? ( (__DATE__[1] == 'a') ? '1' :       // Jan, Jun or Jul
				((__DATE__[2] == 'n') ? '6' : '7') ) :
		(__DATE__[0] == 'F') ? '2' :                                // Feb 
		(__DATE__[0] == 'M') ? (__DATE__[2] == 'r') ? '3' : '5' :   // Mar or May
		(__DATE__[0] == 'A') ? (__DATE__[1] == 'p') ? '4' : '8' :   // Apr or Aug
		(__DATE__[0] == 'S') ? '9' :                                // Sep
		(__DATE__[0] == 'O') ? '0' :                                // Oct
		(__DATE__[0] == 'N') ? '1' :                                // Nov
		(__DATE__[0] == 'D') ? '2' :                                // Dec
		0,
		'-',
		// First day letter, replace space with digit
		__DATE__[4]==' ' ? '0' : __DATE__[4],
		// Second day letter
		__DATE__[5],
		'T',
		// HH:MM:SS
		__TIME__[0], __TIME__[1], __TIME__[2], __TIME__[3], __TIME__[4], __TIME__[5], __TIME__[6], __TIME__[7],
		'\0'
	};

	return dt;
}
