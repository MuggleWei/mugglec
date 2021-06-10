/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_SLEEP_H_
#define MUGGLE_C_SLEEP_H_

#include "muggle/c/base/macro.h"

EXTERN_C_BEGIN

/*
 * sleep this thread for ms milliseconds
 * RETURN: 0 represent success, interupt by signal will return MUGGLE_ERR_INTERRUPT
 * */
MUGGLE_C_EXPORT
int muggle_msleep(unsigned long ms);

EXTERN_C_END

#endif
