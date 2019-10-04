/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_DL_H_
#define MUGGLE_C_DL_H_

#include "muggle/c/base/macro.h"

EXTERN_C_BEGIN

/*
 * loads the dynamic shared object
 * */
MUGGLE_CC_EXPORT
void* muggle_dl_load(const char* path);

/*
 * takes a "handle" of a dynamic loaded shared object returned by 
 * muggle_dl_load along with a null-terminated symbol name
 * */
MUGGLE_CC_EXPORT
void* muggle_dl_sym(void *handle, const char *symbol);

/*
 * decrements the reference count on the dynamically loaded shared 
 * object referred to by handle
 * */
MUGGLE_CC_EXPORT
int muggle_dl_close(void *handle);

EXTERN_C_END

#endif
