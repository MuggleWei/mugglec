/******************************************************************************
 *  @file         dl.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-21
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec dynamic/shared library handle
 *****************************************************************************/

#ifndef MUGGLE_C_DL_H_
#define MUGGLE_C_DL_H_

#include "muggle/c/base/macro.h"

EXTERN_C_BEGIN

/**
 * @brief loads the dynamic shared object
 *
 * @param path filepath of dynamic/shared library
 *
 * @return handle for the loaded library
 */
MUGGLE_C_EXPORT
void* muggle_dl_load(const char* path);

/**
 * @brief 
 * takes a "handle" of a dynamic loaded shared object returned by 
 * muggle_dl_load along with a null-terminated symbol name
 *
 * @param handle  library handle
 * @param symbol  symbol in library
 *
 * @return address associated with symbol
 */
MUGGLE_C_EXPORT
void* muggle_dl_sym(void *handle, const char *symbol);

/**
 * @brief 
 * decrements the reference count on the dynamically loaded shared
 * object referred to by handle
 *
 * @param handle library handle
 *
 * @return 
 *     - return 0 on success
 *     - otherwise return error code in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_dl_close(void *handle);

EXTERN_C_END

#endif
