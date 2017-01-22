#include <time.h>
#include <stddef.h>
#include "gtest/gtest.h"
#include "muggle/base/base.h"

TEST(File, Create)
{
	FileHandle fh;
	char tmp[64], file_path[MUGGLE_MAX_PATH] = { 0 };
	int flags, attrs;

	// close log output in file function
	for (int i = 0; i < MUGGLE_LOG_DEFAULT_MAX; ++i)
	{
		LogDefaultSwitch(i, NULL, 0, 0);
	}

	snprintf(tmp, 64, "tmp.%ld", (long)time(NULL));
	ASSERT_TRUE(FileGetAbsolutePath(tmp, file_path));

	flags = MUGGLE_FILE_READ;
	attrs = MUGGLE_FILE_ATTR_OTHER_READ | MUGGLE_FILE_ATTR_OTHER_WRITE | MUGGLE_FILE_ATTR_GRP_READ;
	ASSERT_FALSE(FileHandleOpen(&fh, file_path, flags, attrs));

	flags = MUGGLE_FILE_WRITE | MUGGLE_FILE_CREAT | MUGGLE_FILE_EXCL;
	ASSERT_TRUE(FileHandleOpen(&fh, file_path, flags, attrs));

	// TODO: to be continued...
}