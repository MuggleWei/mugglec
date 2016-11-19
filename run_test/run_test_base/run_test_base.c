#include "muggle/base/base.h"

typedef void(*emptyFunc)();

int main(int argc, char *argv[])
{
	void* dllHandle;
	void* funcHandle;
	emptyFunc func;

	MUGGLE_DEBUG_LOG("Hello world\n");

	dllHandle = DllLoad("RunTestEmpty");
	if (dllHandle == NULL)
	{
		MUGGLE_DEBUG_ERROR("Can't find demo dll\n");
		exit(EXIT_FAILURE);
	}

	funcHandle = DllQueryFunc(dllHandle, "RunTestEmptyFunc");
	if (funcHandle == NULL)
	{
		MUGGLE_DEBUG_ERROR("Can't find demoFunc in dll\n");
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