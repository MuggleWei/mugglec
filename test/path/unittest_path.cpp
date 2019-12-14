#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"

TEST(path, abspath)
{
	int ret;
	char buf[MUGGLE_MAX_PATH], buf2[MUGGLE_MAX_PATH];
	char curdir[MUGGLE_MAX_PATH];

	ret = muggle_os_curdir(curdir, sizeof(curdir));
	ASSERT_EQ(ret, 0);

	muggle_path_join(curdir, "hello", buf2, sizeof(buf2));
	ASSERT_EQ(ret, 0);


	ret = muggle_path_abspath("hello", buf, sizeof(buf));
	ASSERT_EQ(ret, 0);
	EXPECT_STREQ(buf, buf2);

	ret = muggle_path_abspath("hello/../hello", buf, sizeof(buf));
	ASSERT_EQ(ret, 0);
	EXPECT_STREQ(buf, buf2);

	ret = muggle_path_abspath("world/../hello", buf, sizeof(buf));
	ASSERT_EQ(ret, 0);
	EXPECT_STREQ(buf, buf2);
}

TEST(path, basename)
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

TEST(path, dirname)
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

TEST(path, isabs)
{
	EXPECT_FALSE(muggle_path_isabs("hello.txt"));
	EXPECT_TRUE(muggle_path_isabs("/hello.txt"));
	EXPECT_TRUE(muggle_path_isabs("f:/hello.txt"));
}

TEST(path, exists)
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

TEST(path, join)
{
	int ret;
	char buf[MUGGLE_MAX_PATH];

	ret = muggle_path_join("./hello/", "xxx.txt", buf, sizeof(buf));
	ASSERT_EQ(ret, 0);
	ASSERT_STREQ(buf, "./hello/xxx.txt");

	ret = muggle_path_join("./hello", "xxx.txt", buf, sizeof(buf));
	ASSERT_EQ(ret, 0);
	ASSERT_STREQ(buf, "./hello/xxx.txt");

	ret = muggle_path_join("hello", "xxx.txt", buf, sizeof(buf));
	ASSERT_EQ(ret, 0);
	ASSERT_STREQ(buf, "hello/xxx.txt");


	ret = muggle_path_join("./", "../xxx.txt", buf, sizeof(buf));
	ASSERT_EQ(ret, 0);
	ASSERT_STREQ(buf, "./../xxx.txt");

	ret = muggle_path_join(".", "../xxx.txt", buf, sizeof(buf));
	ASSERT_EQ(ret, 0);
	ASSERT_STREQ(buf, "./../xxx.txt");

	ret = muggle_path_join(".", "../xxx.txt", buf, sizeof(buf));
	ASSERT_EQ(ret, 0);
	ASSERT_STREQ(buf, "./../xxx.txt");

	char buf2[8];

	memset(buf2, (int)'?', sizeof(buf2));
	ret = muggle_path_join("/xxx", "yy", buf2, sizeof(buf2));
	ASSERT_EQ(ret, 0);
	ASSERT_STREQ(buf2, "/xxx/yy");

	memset(buf2, (int)'?', sizeof(buf2));
	ret = muggle_path_join("/xxx", "yyy", buf2, sizeof(buf2));
	ASSERT_NE(ret, 0);
}

TEST(path, normpath)
{
	int ret;
	char buf[MUGGLE_MAX_PATH];

	ret = muggle_path_normpath("../../hello", buf, sizeof(buf));
	ASSERT_EQ(ret, 0);
	EXPECT_STREQ(buf, "../../hello");

	ret = muggle_path_normpath("./", buf, sizeof(buf));
	ASSERT_EQ(ret, 0);
	EXPECT_STREQ(buf, "./");

	ret = muggle_path_normpath(".\\", buf, sizeof(buf));
	ASSERT_EQ(ret, 0);
	EXPECT_STREQ(buf, "./");

	ret = muggle_path_normpath("./hello/xxx/../", buf, sizeof(buf));
	ASSERT_EQ(ret, 0);
	EXPECT_STREQ(buf, "hello/");

	ret = muggle_path_normpath("./hello/xxx/..", buf, sizeof(buf));
	ASSERT_EQ(ret, 0);
	EXPECT_STREQ(buf, "hello/");

	ret = muggle_path_normpath("hello/xxx/../../", buf, sizeof(buf));
	ASSERT_EQ(ret, 0);
	EXPECT_STREQ(buf, "./");

	ret = muggle_path_normpath("hello/xxx/../../world", buf, sizeof(buf));
	ASSERT_EQ(ret, 0);
	EXPECT_STREQ(buf, "world");
}
