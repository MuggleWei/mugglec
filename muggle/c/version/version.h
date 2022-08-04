/******************************************************************************
 *  @file         version.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-22
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec version
 *****************************************************************************/

#ifndef MUGGLE_C_VERSION_H_
#define MUGGLE_C_VERSION_H_

#include "muggle/c/base/macro.h"

EXTERN_C_BEGIN

/**
 * @brief get mugglec version
 *
 * @return mugglec version
 */
MUGGLE_C_EXPORT
const char* mugglec_version();

/**
 * @brief get mugglec compile time in preprocessor macro format
 *
 * @return mugglec compile time
 */
MUGGLE_C_EXPORT
const char* mugglec_compile_time();

/**
 * @brief get mugglec compile datetime in format yyyy-mm-ddTHH:MM:SS
 *
 * @return mugglec compile datetime
 */
MUGGLE_C_EXPORT
const char* mugglec_compile_dt();

EXTERN_C_END

#endif
