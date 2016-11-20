#include "muggle/base/base.h"

typedef void(*emptyFunc)();

int main(int argc, char *argv[])
{
	void* dllHandle;
	void* funcHandle;
	emptyFunc func;
	deltaTime dt;
	double ms;
	char buf[MG_MAX_PATH], dll_path[MG_MAX_PATH];
	char dll_name[] = "RunTestEmpty", dll_name2[] = "../lib/libRunTestEmpty";
	size_t len;

	// log
	MUGGLE_DEBUG_LOG("Hello world\n");

	// sleep and delta time
	MUGGLE_DEBUG_LOG("I will sleep 3000ms\n");

	DTStart(&dt);
	MSleep(3000);
	DTEnd(&dt);

	ms = DTGetElapsedMilliseconds(&dt);
	MUGGLE_DEBUG_LOG("real sleep time: %fms\n", ms);

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