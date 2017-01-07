/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "muggle/base/dll.h"
#include "muggle/base/log.h"
#include "muggle/base/str.h"

#if MUGGLE_PLATFORM_WINDOWS

#include <windows.h>

void* DllLoad(const char* name)
{
	const char* suffix = ".dll";
	char buf[MUGGLE_MAX_PATH];
	memset(buf, 0, MUGGLE_MAX_PATH);

	size_t len = strlen(name);
	memcpy(buf, name, len);
	if (!StrEndsWith(buf, suffix))
	{
		memcpy(buf + len, suffix, 4);
	}

	// convert to utf16 characters
	WCHAR unicode_buf[MUGGLE_MAX_PATH] = { 0 };
	MultiByteToWideChar(CP_UTF8, 0, buf, -1, unicode_buf, MUGGLE_MAX_PATH);

	return (void*)LoadLibraryW(unicode_buf);
}
void* DllQueryFunc(void* dll, const char* func_name)
{
	MUGGLE_ASSERT_MSG(dll != NULL, "Dynamic library is null");
	return (void*)GetProcAddress((HMODULE)dll, func_name);
}
bool DllFree(void* dll)
{
	MUGGLE_ASSERT_MSG(dll != NULL, "Dynamic library is null");
	return (bool)FreeLibrary((HMODULE)dll);
}

#else

#include <dlfcn.h>
#include <string.h>

void* DllLoad(const char* name)
{
	const char* suffix = ".so";
	char buf[MUGGLE_MAX_PATH];
	memset(buf, 0, MUGGLE_MAX_PATH);

	size_t len = strlen(name);
	memcpy(buf, name, len);
	if (!StrEndsWith(buf, suffix))
	{
		memcpy(buf + len, suffix, 3);
	}	

	return dlopen(buf, RTLD_NOW);
}
void* DllQueryFunc(void* dll, const char* func_name)
{
	MUGGLE_ASSERT_MSG(dll != NULL, "Dynamic library is null");
	return dlsym(dll, func_name);
}
bool DllFree(void* dll)
{
	MUGGLE_ASSERT_MSG(dll != NULL, "Dynamic library is null");
	return dlclose(dll) == 0;
}

#endif