/******************************************************************************
 *  @file         sleep.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-15
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec sleep header file
 *****************************************************************************/

#ifndef MUGGLE_C_SLEEP_H_
#define MUGGLE_C_SLEEP_H_

#include "muggle/c/base/macro.h"

EXTERN_C_BEGIN

/**
 * @brief sleep current thread for ms milliseconds
 *
 * @param ms
 *
 * @return
 *     0 - success
 *     interupt by signal will return MUGGLE_ERR_INTERRUPT
 */
MUGGLE_C_EXPORT
int muggle_msleep(unsigned long ms);

EXTERN_C_END

#endif
