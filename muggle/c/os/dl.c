/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "dl.h"

#if MUGGLE_PLATFORM_WINDOWS

#include <windows.h>
#include <string.h>
#include "muggle/c/base/err.h"

void* muggle_dl_load(const char* path)
{
	char buf[MUGGLE_MAX_PATH];
	buf[MUGGLE_MAX_PATH - 1] = '\0';
	strncpy(buf, path, MUGGLE_MAX_PATH - 1);

	// convert to utf16 characters
	WCHAR unicode_buf[MUGGLE_MAX_PATH] = { 0 };
	MultiByteToWideChar(CP_UTF8, 0, buf, -1, unicode_buf, MUGGLE_MAX_PATH);

	return (void*)LoadLibraryW(unicode_buf);
}

void* muggle_dl_sym(void *handle, const char *symbol)
{
	return (void*)GetProcAddress((HMODULE)handle, symbol);
}

int muggle_dl_close(void *handle)
{
	return FreeLibrary((HMODULE)handle) ? MUGGLE_OK: MUGGLE_ERR_SYS_CALL;
}

#else

#include <dlfcn.h>
#include <string.h>
#include "muggle/c/base/err.h"

void* muggle_dl_load(const char* path)
{
	return dlopen(path, RTLD_NOW);
}

void* muggle_dl_sym(void *handle, const char *symbol)
{
	return dlsym(handle, symbol);
}

int muggle_dl_close(void *handle)
{
	return dlclose(handle) == 0 ? MUGGLE_OK : MUGGLE_ERR_SYS_CALL;
}

#endif
