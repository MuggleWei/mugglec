/******************************************************************************
 *  @file         cpu_cycle.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-22
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec cpu cycle
 *****************************************************************************/

#ifndef MUGGLE_C_CPU_CYCLE_H_
#define MUGGLE_C_CPU_CYCLE_H_

#include "muggle/c/base/macro.h"
#include <stdint.h>

EXTERN_C_BEGIN

/**
 * @brief get cpu cycle
 *
 * @return number of clock cycles since the last reset
 */
MUGGLE_C_EXPORT
uint64_t muggle_get_cpu_cycle();

EXTERN_C_END

#endif
