/******************************************************************************
 *  @file         win_gmtime.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-09-23
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec getopt in windows
 *****************************************************************************/

#ifndef MUGGLE_C_GETOPT_H_
#define MUGGLE_C_GETOPT_H_

#include "muggle/c/base/macro.h"

EXTERN_C_BEGIN

#if MUGGLE_PLATFORM_WINDOWS

extern MUGGLE_C_EXPORT int opterr; /* if error message should be printed */
extern MUGGLE_C_EXPORT int optind; /* index into parent argv vector */
extern MUGGLE_C_EXPORT int optopt; /* character checked for validity */
extern MUGGLE_C_EXPORT int optreset; /* reset getopt */
extern MUGGLE_C_EXPORT char *optarg; /* argument associated with option */

struct option
{
	const char *name;
	int has_arg;
	int *flag;
	int val;
};

#define no_argument       0
#define required_argument 1
#define optional_argument 2

MUGGLE_C_EXPORT
int getopt(int nargc, char* const nargv[], const char *ostr);

MUGGLE_C_EXPORT
int getopt_long(
	int nargc, char **nargv,
	const char *options,
	const struct option *long_options,
	int *index)

#else

#include <unistd.h>  // include getopt function in *nix
#include <getopt.h>  // include getopt_long, getopt_long_only in *nix

#endif // MUGGLE_PLATFORM_WINDOWS

EXTERN_C_END

#endif // !MUGGLE_C_GETOPT_H_
