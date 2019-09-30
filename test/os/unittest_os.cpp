#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"

TEST(os, path_abspath)
{
	// TODO:
}

TEST(os, path_basename)
{
	int ret;
	char buf[MUGGLE_MAX_PATH];

	const char *path[] = {
		"/tmp/hello world.txt",
		"/hello world.txt",
		"f:\\hello world.txt",
		"f:\\tmp\\hello world.txt",
		"f:/hello world.txt",
		"f:/tmp/hello world.txt"
	};

	const char *basename[] = {
		"hello world.txt",
		"hello world.txt",
		"hello world.txt",
		"hello world.txt",
		"hello world.txt",
		"hello world.txt"
	};

	for (int i = 0; i < (int)(sizeof(path) / sizeof(path[0])); i++)
	{
		ret = muggle_path_basename(path[i], buf, sizeof(buf));
		EXPECT_EQ(ret, 0);
		EXPECT_STREQ(buf, basename[i]);
	}
}

TEST(os, path_dirname)
{
	int ret;
	char buf[MUGGLE_MAX_PATH];

	const char *path[] = {
		"/tmp/hello world.txt",
		"/hello world.txt",
		"f:\\hello world.txt",
		"f:\\tmp\\hello world.txt",
		"f:/hello world.txt",
		"f:/tmp/hello world.txt"
	};

	const char *dirname[] = {
		"/tmp",
		"/",
		"f:\\",
		"f:\\tmp",
		"f:/",
		"f:/tmp",
	};

	for (int i = 0; i < (int)(sizeof(path) / sizeof(path[0])); i++)
	{
		ret = muggle_path_dirname(path[i], buf, sizeof(buf));
		EXPECT_EQ(ret, 0);
		EXPECT_STREQ(buf, dirname[i]);
	}
}

TEST(os, process_path)
{
	char path[MUGGLE_MAX_PATH] = {0};
	int ret;

	ret = muggle_os_process_path(path, sizeof(path));
	ASSERT_EQ(ret, 0);
	ASSERT_TRUE(path[0] != '\0');
}

TEST(os, curdir)
{
	char path[MUGGLE_MAX_PATH] = {0};
	int ret;

	ret = muggle_os_curdir(path, sizeof(path));
	ASSERT_EQ(ret, 0);
	ASSERT_TRUE(path[0] != '\0');
}

TEST(os, chdir)
{
	char process_path[MUGGLE_MAX_PATH] = {0};
	char working_path[MUGGLE_MAX_PATH] = {0};
#if MUGGLE_PLATFORM_WINDOWS
	const char *target_dir = "c:/";
#else
	const char *target_dir = "/";
#endif
	int ret;

	ret = muggle_os_process_path(process_path, sizeof(process_path));
	ASSERT_EQ(ret, 0);

	char process_dir[MUGGLE_MAX_PATH];
	ret = muggle_path_dirname(process_path, process_dir, sizeof(process_dir));
	ASSERT_EQ(ret, 0);

	ret = muggle_os_chdir(target_dir);
	ASSERT_EQ(ret, 0);

	muggle_os_curdir(working_path, sizeof(working_path));
	EXPECT_STREQ(working_path, target_dir);

	ret = muggle_os_chdir(process_dir);
	ASSERT_EQ(ret, 0);
	ret = muggle_os_curdir(working_path, sizeof(working_path));
	ASSERT_EQ(ret, 0);
	EXPECT_STREQ(working_path, process_dir);
}

