#include "muggle/base/base.h"

typedef void(*emptyFunc)();

void TestFileHandle()
{
	FileHandle fh;
	int flags, attr;
	char buf1[] = "hello world", buf2[64] = { 0 };
	long num_read;

	flags = 
		MUGGLE_FILE_WRITE | MUGGLE_FILE_READ | MUGGLE_FILE_APPEND | 
		MUGGLE_FILE_CREAT | MUGGLE_FILE_EXCL;
	attr = MUGGLE_FILE_ATTR_USER_READ | MUGGLE_FILE_ATTR_USER_WRITE;
	fh = FileHandleOpen("tmp.txt", flags, attr);
	if (!FileHandleIsValid(fh))
	{
		flags &= ~MUGGLE_FILE_EXCL;
		flags |= MUGGLE_FILE_TRUNC;
		fh = FileHandleOpen("tmp.txt", flags, attr);
		if (!FileHandleIsValid(fh))
		{
			MUGGLE_DEBUG_ERROR("Can't get file handle of tmp.txt\n");
			return;
		}
		else
		{
			MUGGLE_DEBUG_INFO("Open tmp.txt and truncate\n");
		}
	}
	else
	{
		MUGGLE_DEBUG_INFO("I create tmp.txt\n");
	}

	if (FileHandleWrite(fh, buf1, (long)strlen(buf1)) == -1)
	{
		MUGGLE_DEBUG_ERROR("Failed write into file handle\n");
		return;
	}

	if (!FileHandleClose(fh))
	{
		MUGGLE_DEBUG_ERROR("Failed close file handle\n");
		return;
	}

	flags &= ~(MUGGLE_FILE_EXCL | MUGGLE_FILE_TRUNC);
	fh = FileHandleOpen("tmp.txt", flags, attr);
	if (!FileHandleIsValid(fh))
	{
		MUGGLE_DEBUG_ERROR("Can't open tmp.txt\n");
		return;
	}

	num_read = FileHandleRead(fh, buf2, 64);
	if (num_read == -1)
	{
		MUGGLE_DEBUG_ERROR("Failed read data from file handle\n");
		return;
	}
	buf2[num_read] = '\0';
	MUGGLE_INFO("Read data: %s\n", buf2);

	if (FileHandleSeek(fh, 6L, MUGGLE_FILE_SEEK_BEGIN) == -1)
	{
		MUGGLE_DEBUG_ERROR("Failed seek in file handle\n");
		return;
	}
	num_read = FileHandleRead(fh, buf2, 64);
	if (num_read == -1)
	{
		MUGGLE_DEBUG_ERROR("Failed read data file handle\n");
		return;
	}
	buf2[num_read] = '\0';
	MUGGLE_INFO("Read data: %s\n", buf2);

	if (!FileHandleClose(fh))
	{
		MUGGLE_DEBUG_ERROR("Failed close file handle\n");
		return;
	}
}

int main(int argc, char *argv[])
{
	void* dllHandle;
	void* funcHandle;
	emptyFunc func;
	deltaTime dt;
	double ms;
	char buf[MUGGLE_MAX_PATH];
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

	// file handle
	TestFileHandle();

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