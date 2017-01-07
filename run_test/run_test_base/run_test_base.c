#include "muggle/base/base.h"

typedef void(*emptyFunc)();

int main(int argc, char *argv[])
{
	void* dllHandle;
	void* funcHandle;
	emptyFunc func;
	deltaTime dt;
	double ms;
	char buf[MG_MAX_PATH];
	char dll_name[] = "RunTestEmpty", dll_name2[] = "../lib/libRunTestEmpty";
	size_t len;
	int ival;
	unsigned int uival;
	long lval;
	unsigned long ulval;
	float fval;

	// platform macro
#if MUGGLE_PLATFORM_WIN64
	MUGGLE_DEBUG_INFO("Win64\n");
#elif MUGGLE_PLATFORM_WINDOWS
	MUGGLE_DEBUG_INFO("Win32\n");
#elif MUGGLE_PLATFORM_LINUX
	MUGGLE_DEBUG_INFO("Linux\n");
#elif MUGGLE_PLATFORM_APPLE
	MUGGLE_DEBUG_INFO("Apple\n");
#elif MUGGLE_PLATFORM_UNIX
	MUGGLE_DEBUG_INFO("Unix\n");
#endif

	// log
	MUGGLE_DEBUG_INFO("Hello world\n");

	// sleep and delta time
	MUGGLE_DEBUG_INFO("I will sleep 3000ms\n");

	DTStart(&dt);
	MSleep(3000);
	DTEnd(&dt);

	ms = DTGetElapsedMilliseconds(&dt);
	MUGGLE_DEBUG_INFO("real sleep time: %fms\n", ms);

	// string convert
	if (StrToi("-1024", &ival, 0))
	{
		MUGGLE_DEBUG_INFO("int: %d\n", ival);
	}
	if (StrToui("1024", &uival, 0))
	{
		MUGGLE_DEBUG_INFO("uint: %d\n", uival);
	}
	if (StrTol("-1024", &lval, 0))
	{
		MUGGLE_DEBUG_INFO("long: %ld\n", lval);
	}
	if (StrToul("102400", &ulval, 0))
	{
		MUGGLE_DEBUG_INFO("ulong: %uld\n", ulval);
	}
	if (StrTof("5.2", &fval))
	{
		MUGGLE_DEBUG_INFO("float: %f\n", fval);
	}

	// file and dll
	FileGetProcessPath(buf);
	FileGetDirectory(buf, buf);
	len = strlen(buf);
	memcpy(buf + len, dll_name, strlen(dll_name));
	buf[len+strlen(dll_name)] = '\0';

	dllHandle = DllLoad(buf);
	if (dllHandle == NULL)
	{
		memcpy(buf + len, dll_name2, strlen(dll_name2));
		buf[len+strlen(dll_name2)] = '\0';
		dllHandle = DllLoad(buf);
	}
	if (dllHandle == NULL)
	{
		MUGGLE_DEBUG_ERROR("Can't find demo dll\n");
		exit(EXIT_FAILURE);
	}

	funcHandle = DllQueryFunc(dllHandle, "RunTestEmptyFunc");
	if (funcHandle == NULL)
	{
		MUGGLE_DEBUG_ERROR("Can't find function in dll\n");
		exit(EXIT_FAILURE);
	}

	func = (emptyFunc)funcHandle;
	func();

	if (!DllFree(dllHandle))
	{
		MUGGLE_DEBUG_ERROR("Failed in free dll\n");
		exit(EXIT_FAILURE);
	}

	return 0;
}