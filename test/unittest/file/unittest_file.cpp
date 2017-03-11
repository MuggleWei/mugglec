#include <time.h>
#include <stddef.h>
#include "gtest/gtest.h"
#include "muggle/base_c/base_c.h"

TEST(File, OpenCreateDelete)
{
	MuggleFile fh;
	char tmp[64], file_path[MUGGLE_MAX_PATH] = { 0 };
	int flags, attrs;

	// close log output in file function
	for (int i = 0; i < MUGGLE_LOG_DEFAULT_MAX; ++i)
	{
		LogDefaultSwitch(i, NULL, 0, 0);
	}

	snprintf(tmp, 64, "tmp_ocd.%ld", (long)time(NULL));
	ASSERT_TRUE(MuggleGetAbsolutePath(tmp, file_path));

	if (MuggleIsFileExist(file_path))
	{
		MuggleDeleteFile(file_path);
	}

	flags = MUGGLE_FILE_READ;
	attrs = MUGGLE_PERM_USER_READ | MUGGLE_PERM_USER_WRITE| MUGGLE_PERM_GRP_READ;
	ASSERT_FALSE(MuggleFileOpen(&fh, file_path, flags, attrs));
	ASSERT_FALSE(MuggleFileIsValid(&fh));

	flags = MUGGLE_FILE_WRITE | MUGGLE_FILE_CREAT | MUGGLE_FILE_EXCL;
	ASSERT_TRUE(MuggleFileOpen(&fh, file_path, flags, attrs));
	ASSERT_TRUE(MuggleFileIsValid(&fh));
	ASSERT_TRUE(MuggleFileClose(&fh));

	int false_flags_array[] = {
		MUGGLE_FILE_EXCL | MUGGLE_FILE_CREAT | MUGGLE_FILE_READ,
		MUGGLE_FILE_EXCL | MUGGLE_FILE_CREAT | MUGGLE_FILE_WRITE,
		MUGGLE_FILE_EXCL | MUGGLE_FILE_CREAT | MUGGLE_FILE_APPEND,
		MUGGLE_FILE_EXCL | MUGGLE_FILE_CREAT | MUGGLE_FILE_READ | MUGGLE_FILE_WRITE,
		MUGGLE_FILE_EXCL | MUGGLE_FILE_CREAT | MUGGLE_FILE_READ | MUGGLE_FILE_APPEND,
		MUGGLE_FILE_EXCL | MUGGLE_FILE_CREAT | MUGGLE_FILE_WRITE | MUGGLE_FILE_APPEND,
		MUGGLE_FILE_EXCL | MUGGLE_FILE_CREAT | MUGGLE_FILE_READ | MUGGLE_FILE_WRITE | MUGGLE_FILE_APPEND,
		MUGGLE_FILE_EXCL | MUGGLE_FILE_CREAT | MUGGLE_FILE_TRUNC | MUGGLE_FILE_READ,
		MUGGLE_FILE_EXCL | MUGGLE_FILE_CREAT | MUGGLE_FILE_TRUNC | MUGGLE_FILE_WRITE,
		MUGGLE_FILE_EXCL | MUGGLE_FILE_CREAT | MUGGLE_FILE_TRUNC | MUGGLE_FILE_APPEND,
		MUGGLE_FILE_EXCL | MUGGLE_FILE_CREAT | MUGGLE_FILE_TRUNC | MUGGLE_FILE_READ | MUGGLE_FILE_WRITE,
		MUGGLE_FILE_EXCL | MUGGLE_FILE_CREAT | MUGGLE_FILE_TRUNC | MUGGLE_FILE_READ | MUGGLE_FILE_APPEND,
		MUGGLE_FILE_EXCL | MUGGLE_FILE_CREAT | MUGGLE_FILE_TRUNC | MUGGLE_FILE_WRITE | MUGGLE_FILE_APPEND,
		MUGGLE_FILE_EXCL | MUGGLE_FILE_CREAT | MUGGLE_FILE_TRUNC | MUGGLE_FILE_READ | MUGGLE_FILE_WRITE | MUGGLE_FILE_APPEND,
	};
	for (int i = 0; i < (int)sizeof(false_flags_array) / (int)sizeof(false_flags_array[0]); ++i)
	{
		ASSERT_FALSE(MuggleFileOpen(&fh, file_path, false_flags_array[i], attrs));
		ASSERT_FALSE(MuggleFileIsValid(&fh));
	}

	int true_flags_array[] = {
		MUGGLE_FILE_READ,
		MUGGLE_FILE_WRITE,
		MUGGLE_FILE_APPEND,
		MUGGLE_FILE_READ | MUGGLE_FILE_WRITE,
		MUGGLE_FILE_READ | MUGGLE_FILE_APPEND,
		MUGGLE_FILE_WRITE | MUGGLE_FILE_APPEND,
		MUGGLE_FILE_READ | MUGGLE_FILE_WRITE | MUGGLE_FILE_APPEND,
		MUGGLE_FILE_CREAT | MUGGLE_FILE_READ,
		MUGGLE_FILE_CREAT | MUGGLE_FILE_WRITE,
		MUGGLE_FILE_CREAT | MUGGLE_FILE_APPEND,
		MUGGLE_FILE_CREAT | MUGGLE_FILE_READ | MUGGLE_FILE_WRITE,
		MUGGLE_FILE_CREAT | MUGGLE_FILE_READ | MUGGLE_FILE_APPEND,
		MUGGLE_FILE_CREAT | MUGGLE_FILE_WRITE | MUGGLE_FILE_APPEND,
		MUGGLE_FILE_CREAT | MUGGLE_FILE_READ | MUGGLE_FILE_WRITE | MUGGLE_FILE_APPEND,
		MUGGLE_FILE_TRUNC | MUGGLE_FILE_READ,
		MUGGLE_FILE_TRUNC | MUGGLE_FILE_WRITE,
		MUGGLE_FILE_TRUNC | MUGGLE_FILE_APPEND,
		MUGGLE_FILE_TRUNC | MUGGLE_FILE_READ | MUGGLE_FILE_WRITE,
		MUGGLE_FILE_TRUNC | MUGGLE_FILE_READ | MUGGLE_FILE_APPEND,
		MUGGLE_FILE_TRUNC | MUGGLE_FILE_WRITE | MUGGLE_FILE_APPEND,
		MUGGLE_FILE_TRUNC | MUGGLE_FILE_READ | MUGGLE_FILE_WRITE | MUGGLE_FILE_APPEND,
		MUGGLE_FILE_TRUNC | MUGGLE_FILE_CREAT | MUGGLE_FILE_READ,
		MUGGLE_FILE_TRUNC | MUGGLE_FILE_CREAT | MUGGLE_FILE_WRITE,
		MUGGLE_FILE_TRUNC | MUGGLE_FILE_CREAT | MUGGLE_FILE_APPEND,
		MUGGLE_FILE_TRUNC | MUGGLE_FILE_CREAT | MUGGLE_FILE_READ | MUGGLE_FILE_WRITE,
		MUGGLE_FILE_TRUNC | MUGGLE_FILE_CREAT | MUGGLE_FILE_READ | MUGGLE_FILE_APPEND,
		MUGGLE_FILE_TRUNC | MUGGLE_FILE_CREAT | MUGGLE_FILE_WRITE | MUGGLE_FILE_APPEND,
		MUGGLE_FILE_TRUNC | MUGGLE_FILE_CREAT | MUGGLE_FILE_READ | MUGGLE_FILE_WRITE | MUGGLE_FILE_APPEND,
	};
	for (int i = 0; i < (int)sizeof(true_flags_array) / (int)sizeof(true_flags_array[0]); ++i)
	{
		ASSERT_TRUE(MuggleFileOpen(&fh, file_path, true_flags_array[i], attrs));
		ASSERT_TRUE(MuggleFileIsValid(&fh));
		ASSERT_TRUE(MuggleFileClose(&fh));
	}

	ASSERT_TRUE(MuggleDeleteFile(file_path));
}

TEST(File, ReadWriteAppend)
{
	const int buf_size = 64;
	MuggleFile fh;
	char tmp[buf_size], file_path[MUGGLE_MAX_PATH] = { 0 };
	char buf[buf_size];
	int flags, attrs;
	const char *str1 = "hello world";
	const char *str2 = "append";
	long len1 = (long)strlen(str1);
	long len2 = (long)strlen(str2);

	// close log output in file function
	for (int i = 0; i < MUGGLE_LOG_DEFAULT_MAX; ++i)
	{
		LogDefaultSwitch(i, NULL, 0, 0);
	}

	snprintf(tmp, 64, "tmp_rwa.%ld", (long)time(NULL));
	ASSERT_TRUE(MuggleGetAbsolutePath(tmp, file_path));

	if (MuggleIsFileExist(file_path))
	{
		MuggleDeleteFile(file_path);
	}

	// create file and write into a string
	flags = MUGGLE_FILE_READ | MUGGLE_FILE_WRITE | MUGGLE_FILE_CREAT | MUGGLE_FILE_EXCL;
	attrs = MUGGLE_PERM_USER_READ | MUGGLE_PERM_USER_WRITE | MUGGLE_PERM_GRP_READ;
	ASSERT_TRUE(MuggleFileOpen(&fh, file_path, flags, attrs));
	ASSERT_EQ(MuggleFileRead(&fh, buf, buf_size), 0);
	ASSERT_EQ(MuggleFileWrite(&fh, (const void*)str1, len1), len1);
	ASSERT_TRUE(MuggleFileClose(&fh));

	// read the string previously written into file
	int read_flags_array[] = {
		MUGGLE_FILE_READ,
		MUGGLE_FILE_READ | MUGGLE_FILE_WRITE,
		MUGGLE_FILE_READ | MUGGLE_FILE_APPEND,
		MUGGLE_FILE_READ | MUGGLE_FILE_WRITE | MUGGLE_FILE_APPEND,
		MUGGLE_FILE_CREAT | MUGGLE_FILE_READ,
		MUGGLE_FILE_CREAT | MUGGLE_FILE_READ | MUGGLE_FILE_WRITE,
		MUGGLE_FILE_CREAT | MUGGLE_FILE_READ | MUGGLE_FILE_APPEND,
		MUGGLE_FILE_CREAT | MUGGLE_FILE_READ | MUGGLE_FILE_WRITE | MUGGLE_FILE_APPEND,
	};
	for (int i = 0; i < (int)sizeof(read_flags_array) / (int)sizeof(read_flags_array[0]); ++i)
	{
		flags = read_flags_array[i];
		ASSERT_TRUE(MuggleFileOpen(&fh, file_path, flags, attrs));
		ASSERT_EQ(MuggleFileRead(&fh, buf, len1), len1);
		buf[len1] = '\0';
		ASSERT_STREQ(buf, str1);
		ASSERT_TRUE(MuggleFileClose(&fh));
	}

	// write file
	int write_flags_array[] = {
		MUGGLE_FILE_WRITE,
		MUGGLE_FILE_READ | MUGGLE_FILE_WRITE,
		MUGGLE_FILE_CREAT | MUGGLE_FILE_WRITE,
		MUGGLE_FILE_CREAT | MUGGLE_FILE_READ | MUGGLE_FILE_WRITE,
	};
	ASSERT_GT(len1, 1);
	for (int i = 0; i < (int)sizeof(write_flags_array) / (int)sizeof(write_flags_array[0]); ++i)
	{
		ASSERT_TRUE(MuggleFileOpen(&fh, file_path, write_flags_array[i], attrs));
		ASSERT_EQ(MuggleFileWrite(&fh, str1, len1), len1);
		ASSERT_TRUE(MuggleFileClose(&fh));

		ASSERT_TRUE(MuggleFileOpen(&fh, file_path, write_flags_array[i], attrs));
		ASSERT_EQ(MuggleFileWrite(&fh, "w", 1), 1);
		ASSERT_TRUE(MuggleFileClose(&fh));

		flags = MUGGLE_FILE_READ;
		memset(buf, 0, buf_size);
		ASSERT_TRUE(MuggleFileOpen(&fh, file_path, flags, attrs));
		ASSERT_EQ(MuggleFileRead(&fh, buf, 1), 1);
		ASSERT_EQ(buf[0], 'w');
		ASSERT_EQ(MuggleFileRead(&fh, buf, len1 - 1), len1 - 1);
		buf[len1 - 1] = '\0';
		ASSERT_STREQ(buf, str1 + 1);
		ASSERT_TRUE(MuggleFileClose(&fh));
	}

	// truncate file
	int trunc_flags_array[] = {
		MUGGLE_FILE_TRUNC | MUGGLE_FILE_READ,
		MUGGLE_FILE_TRUNC | MUGGLE_FILE_READ | MUGGLE_FILE_WRITE,
		MUGGLE_FILE_TRUNC | MUGGLE_FILE_READ | MUGGLE_FILE_APPEND,
		MUGGLE_FILE_TRUNC | MUGGLE_FILE_READ | MUGGLE_FILE_WRITE | MUGGLE_FILE_APPEND,
		MUGGLE_FILE_TRUNC | MUGGLE_FILE_CREAT | MUGGLE_FILE_READ,
		MUGGLE_FILE_TRUNC | MUGGLE_FILE_CREAT | MUGGLE_FILE_READ | MUGGLE_FILE_WRITE,
		MUGGLE_FILE_TRUNC | MUGGLE_FILE_CREAT | MUGGLE_FILE_READ | MUGGLE_FILE_APPEND,
		MUGGLE_FILE_TRUNC | MUGGLE_FILE_CREAT | MUGGLE_FILE_READ | MUGGLE_FILE_WRITE | MUGGLE_FILE_APPEND,
	};
	for (int i = 0; i < (int)sizeof(trunc_flags_array) / (int)sizeof(trunc_flags_array[0]); ++i)
	{
		flags = MUGGLE_FILE_WRITE;
		ASSERT_TRUE(MuggleFileOpen(&fh, file_path, flags, attrs));
		ASSERT_EQ(MuggleFileWrite(&fh, (const void*)str1, len1), len1);
		ASSERT_TRUE(MuggleFileClose(&fh));

		flags = trunc_flags_array[i];
		ASSERT_TRUE(MuggleFileOpen(&fh, file_path, flags, attrs));
		ASSERT_EQ(MuggleFileRead(&fh, buf, len1), 0);
		ASSERT_TRUE(MuggleFileClose(&fh));
	}

	// append file
	int append_flags_array[] = {
		MUGGLE_FILE_APPEND,
		MUGGLE_FILE_APPEND | MUGGLE_FILE_READ,
		MUGGLE_FILE_APPEND | MUGGLE_FILE_WRITE,
		MUGGLE_FILE_APPEND | MUGGLE_FILE_CREAT,
		MUGGLE_FILE_APPEND | MUGGLE_FILE_CREAT | MUGGLE_FILE_READ,
		MUGGLE_FILE_APPEND | MUGGLE_FILE_CREAT | MUGGLE_FILE_WRITE,
		MUGGLE_FILE_APPEND | MUGGLE_FILE_TRUNC,
		MUGGLE_FILE_APPEND | MUGGLE_FILE_TRUNC | MUGGLE_FILE_READ,
		MUGGLE_FILE_APPEND | MUGGLE_FILE_TRUNC | MUGGLE_FILE_WRITE,
		MUGGLE_FILE_APPEND | MUGGLE_FILE_CREAT | MUGGLE_FILE_TRUNC,
		MUGGLE_FILE_APPEND | MUGGLE_FILE_CREAT | MUGGLE_FILE_TRUNC | MUGGLE_FILE_READ,
		MUGGLE_FILE_APPEND | MUGGLE_FILE_CREAT | MUGGLE_FILE_TRUNC | MUGGLE_FILE_WRITE,
	};
	for (int i = 0; i < (int)sizeof(append_flags_array) / (int)sizeof(append_flags_array[0]); ++i)
	{
		flags = MUGGLE_FILE_WRITE | MUGGLE_FILE_TRUNC;
		ASSERT_TRUE(MuggleFileOpen(&fh, file_path, flags, attrs));
		ASSERT_EQ(MuggleFileWrite(&fh, (const void*)str1, len1), len1);
		ASSERT_TRUE(MuggleFileClose(&fh));

		if (!(append_flags_array[i] & MUGGLE_FILE_TRUNC) && (append_flags_array[i] & MUGGLE_FILE_READ))
		{
			ASSERT_TRUE(MuggleFileOpen(&fh, file_path, append_flags_array[i], attrs));
			ASSERT_EQ(MuggleFileRead(&fh, buf, len1), len1);
			buf[len1] = '\0';
			ASSERT_STREQ(buf, str1);
			ASSERT_TRUE(MuggleFileClose(&fh));
		}

		ASSERT_TRUE(MuggleFileOpen(&fh, file_path, append_flags_array[i], attrs));
		ASSERT_EQ(MuggleFileWrite(&fh, str2, len2), len2);
		ASSERT_TRUE(MuggleFileClose(&fh));

		flags = MUGGLE_FILE_READ;
		ASSERT_TRUE(MuggleFileOpen(&fh, file_path, flags, attrs));
		if (append_flags_array[i] & MUGGLE_FILE_TRUNC)
		{
			ASSERT_EQ(MuggleFileRead(&fh, buf, len2), len2);
			buf[len2] = '\0';
			ASSERT_STREQ(buf, str2);
			ASSERT_EQ(MuggleFileRead(&fh, buf, 1), 0);
		}
		else
		{
			ASSERT_EQ(MuggleFileRead(&fh, buf, len1), len1);
			buf[len1] = '\0';
			ASSERT_STREQ(buf, str1);
			ASSERT_EQ(MuggleFileRead(&fh, buf, len2), len2);
			buf[len2] = '\0';
			ASSERT_STREQ(buf, str2);
			ASSERT_EQ(MuggleFileRead(&fh, buf, 1), 0);
		}
		ASSERT_TRUE(MuggleFileClose(&fh));
	}

	ASSERT_TRUE(MuggleDeleteFile(file_path));
}

TEST(File, Seek)
{
	const int buf_size = 64;
	MuggleFile fh;
	char tmp[buf_size], file_path[MUGGLE_MAX_PATH] = { 0 };
	char buf[buf_size];
	int flags, attrs;
	const char *str[4] = {
		"hello", "foo", "bar", "hey guy"
	};
	long len[4] = { 0 };

	for (int i = 0; i < 4; ++i)
	{
		len[i] = (long)strlen(str[i]);
	}

	// close log output in file function
	for (int i = 0; i < MUGGLE_LOG_DEFAULT_MAX; ++i)
	{
		LogDefaultSwitch(i, NULL, 0, 0);
	}

	snprintf(tmp, 64, "tmp_ocd.%ld", (long)time(NULL));
	ASSERT_TRUE(MuggleGetAbsolutePath(tmp, file_path));

	if (MuggleIsFileExist(file_path))
	{
		MuggleDeleteFile(file_path);
	}

	flags = MUGGLE_FILE_WRITE | MUGGLE_FILE_CREAT | MUGGLE_FILE_EXCL;
	attrs = MUGGLE_PERM_USER_READ | MUGGLE_PERM_USER_WRITE | MUGGLE_PERM_GRP_READ;
	ASSERT_TRUE(MuggleFileOpen(&fh, file_path, flags, attrs));
	ASSERT_EQ(MuggleFileWrite(&fh, str[0], len[0]), len[0]);
	ASSERT_EQ(MuggleFileWrite(&fh, str[1], len[1]), len[1]);
	ASSERT_EQ(MuggleFileWrite(&fh, str[2], len[2]), len[2]);
	ASSERT_TRUE(MuggleFileClose(&fh));

	flags = MUGGLE_FILE_READ;
	ASSERT_TRUE(MuggleFileOpen(&fh, file_path, flags, attrs));
	for (int i = 0; i < 3; ++i)
	{
		ASSERT_EQ(MuggleFileRead(&fh, buf, len[i]), len[i]);
		buf[len[i]] = '\0';
		ASSERT_STREQ(buf, str[i]);
	}
	ASSERT_EQ(MuggleFileSeek(&fh, 0, MUGGLE_FILE_SEEK_BEGIN), 0);
	ASSERT_EQ(MuggleFileSeek(&fh, len[0], MUGGLE_FILE_SEEK_BEGIN), len[0]);
	ASSERT_EQ(MuggleFileSeek(&fh, len[1], MUGGLE_FILE_SEEK_CURRENT), len[1] + len[0]);
	ASSERT_EQ(MuggleFileRead(&fh, buf, len[2]), len[2]);
	buf[len[2]] = '\0';
	ASSERT_STREQ(buf, str[2]);
	ASSERT_EQ(MuggleFileSeek(&fh, -(len[2] + len[1]), MUGGLE_FILE_SEEK_END), len[0]);
	ASSERT_EQ(MuggleFileRead(&fh, buf, len[1]), len[1]);
	buf[len[1]] = '\0';
	ASSERT_STREQ(buf, str[1]);
	ASSERT_TRUE(MuggleFileClose(&fh));

	flags = MUGGLE_FILE_WRITE | MUGGLE_FILE_READ;
	ASSERT_TRUE(MuggleFileOpen(&fh, file_path, flags, attrs));
	ASSERT_EQ(MuggleFileSeek(&fh, 0, MUGGLE_FILE_SEEK_END), len[0] + len[1] + len[2]);
	ASSERT_EQ(MuggleFileWrite(&fh, str[3], len[3]), len[3]);
	ASSERT_TRUE(MuggleFileClose(&fh));

	flags = MUGGLE_FILE_APPEND | MUGGLE_FILE_READ;
	ASSERT_TRUE(MuggleFileOpen(&fh, file_path, flags, attrs));
	ASSERT_EQ(MuggleFileSeek(&fh, len[0] + len[1] + len[2], MUGGLE_FILE_SEEK_BEGIN), len[0] + len[1] + len[2]);
	ASSERT_EQ(MuggleFileRead(&fh, buf, len[3]), len[3]);
	ASSERT_EQ(MuggleFileSeek(&fh, 0, MUGGLE_FILE_SEEK_BEGIN), 0);
	ASSERT_EQ(MuggleFileWrite(&fh, str[0], len[0]), len[0]);
	ASSERT_EQ(MuggleFileSeek(&fh, 0, MUGGLE_FILE_SEEK_CURRENT), len[0] + len[1] + len[2] + len[3] + len[0]);
	ASSERT_EQ(MuggleFileSeek(&fh, 0, MUGGLE_FILE_SEEK_END), len[0] + len[1] + len[2] + len[3] + len[0]);
	ASSERT_TRUE(MuggleFileClose(&fh));

	ASSERT_TRUE(MuggleDeleteFile(file_path));
}