#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"

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
	char origin_working_path[MUGGLE_MAX_PATH] = {0};
	char process_path[MUGGLE_MAX_PATH] = {0};
	char working_path[MUGGLE_MAX_PATH] = {0};
#if MUGGLE_PLATFORM_WINDOWS
	const char *target_dir = "c:\\";
#else
	const char *target_dir = "/";
#endif
	int ret;

	muggle_os_curdir(origin_working_path, sizeof(origin_working_path));

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

	ret = muggle_os_chdir(origin_working_path);
	ASSERT_EQ(ret, 0);
}

TEST(os, mkdir_rmdir)
{
	int ret;
	char buf[MUGGLE_MAX_PATH];
	auto t = time(NULL);
	snprintf(buf, sizeof(buf)-1, "test_os_dir_%lld", (long long)t);

	ret = muggle_os_mkdir(buf);
	ASSERT_EQ(ret, 0);

	ret = muggle_path_exists(buf);
	ASSERT_TRUE(ret);

	ret = muggle_os_rmdir(buf);
	ASSERT_EQ(ret, 0);

	ret = muggle_path_exists(buf);
	ASSERT_FALSE(ret);
}

TEST(os, remove)
{
	int ret;
	char buf[MUGGLE_MAX_PATH];
	auto t = time(NULL);
	snprintf(buf, sizeof(buf)-1, "test_os_file_%lld", (long long)t);

	FILE *fp = fopen(buf, "wb");
	ASSERT_TRUE(fp != NULL);
	fclose(fp);

	ret = muggle_path_exists(buf);
	ASSERT_TRUE(ret);

	ret = muggle_os_remove(buf);
	ASSERT_EQ(ret, 0);

	ret = muggle_path_exists(buf);
	ASSERT_FALSE(ret);
}

TEST(os, rename)
{
	int ret;
	char src[MUGGLE_MAX_PATH], dst[MUGGLE_MAX_PATH];
	auto t = time(NULL);

	snprintf(src, sizeof(src)-1, "test_os_dir_src_%lld", (long long)t);
	ret = muggle_os_mkdir(src);
	ASSERT_EQ(ret, 0);

	snprintf(dst, sizeof(dst)-1, "test_os_dir_dst_%lld", (long long)t);
	ret = muggle_os_rename(src, dst);
	ASSERT_EQ(ret, 0);

	ret = muggle_path_exists(dst);
	ASSERT_TRUE(ret);

	ret = muggle_os_rmdir(dst);
	ASSERT_EQ(ret, 0);

	snprintf(src, sizeof(src)-1, "test_os_file_src_%lld", (long long)t);
	FILE *fp = fopen(src, "wb");
	ASSERT_TRUE(fp != NULL);
	fclose(fp);

	snprintf(dst, sizeof(dst)-1, "test_os_file_dst_%lld", (long long)t);
	ret = muggle_os_rename(src, dst);
	if (ret != 0)
	{
		printf("%s\n", strerror(errno));
	}
	ASSERT_EQ(ret, 0);

	ret = muggle_path_exists(dst);
	ASSERT_TRUE(ret);

	ret = muggle_os_remove(dst);
	ASSERT_EQ(ret, 0);
}
