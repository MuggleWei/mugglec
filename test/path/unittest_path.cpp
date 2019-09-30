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

TEST(os, path_isabs)
{
	EXPECT_FALSE(muggle_path_isabs("hello.txt"));
	EXPECT_TRUE(muggle_path_isabs("/hello.txt"));
	EXPECT_TRUE(muggle_path_isabs("f:/hello.txt"));
}

TEST(os, path_exists)
{
	int ret;
	char process_path[MUGGLE_MAX_PATH] = {0};
	char buf[MUGGLE_MAX_PATH] = {0};

	ret = muggle_os_process_path(process_path, sizeof(process_path));
	ASSERT_EQ(ret, 0);
	ASSERT_TRUE(muggle_path_exists(process_path));

	ret = muggle_path_dirname(process_path, buf, sizeof(buf));
	ASSERT_EQ(ret, 0);
	ASSERT_TRUE(muggle_path_exists(buf));

	// ensure in process dir
	ret = muggle_os_chdir(buf);
	ASSERT_EQ(ret, 0);
	ret = muggle_path_basename(process_path, buf, sizeof(buf));
	ASSERT_EQ(ret, 0);
	ASSERT_TRUE(muggle_path_exists(buf));

	ASSERT_FALSE(muggle_path_exists("/home/not_exist"));

	ret = muggle_path_basename(process_path, buf, sizeof(buf));
	char not_exists[MUGGLE_MAX_PATH];
	snprintf(not_exists, MUGGLE_MAX_PATH, "%s_not_exist", buf);
	ASSERT_FALSE(muggle_path_exists(not_exists));
}
