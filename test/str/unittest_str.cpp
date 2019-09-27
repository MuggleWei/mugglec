#include <time.h>
#include <stddef.h>
#include <limits.h>
#include <float.h>
#include <cmath>
#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"

TEST(str, startswith)
{
	const char *s = "hello world";

	EXPECT_TRUE(muggle_str_startswith(s, "h"));
	EXPECT_TRUE(muggle_str_startswith(s, "he"));
	EXPECT_TRUE(muggle_str_startswith(s, "hel"));
	EXPECT_TRUE(muggle_str_startswith(s, "hell"));
	EXPECT_TRUE(muggle_str_startswith(s, "hello"));
	EXPECT_TRUE(muggle_str_startswith(s, "hello "));
	EXPECT_TRUE(muggle_str_startswith(s, "hello w"));
	EXPECT_TRUE(muggle_str_startswith(s, "hello wo"));
	EXPECT_TRUE(muggle_str_startswith(s, "hello wor"));
	EXPECT_TRUE(muggle_str_startswith(s, "hello worl"));
	EXPECT_TRUE(muggle_str_startswith(s, "hello world"));

	EXPECT_FALSE(muggle_str_startswith(s, ""));
	EXPECT_FALSE(muggle_str_startswith(s, "e"));
	EXPECT_FALSE(muggle_str_startswith(s, "el"));
	EXPECT_FALSE(muggle_str_startswith(s, "ell"));
	EXPECT_FALSE(muggle_str_startswith(s, "ello"));
	EXPECT_FALSE(muggle_str_startswith(s, "ello "));
	EXPECT_FALSE(muggle_str_startswith(s, "ello w"));
	EXPECT_FALSE(muggle_str_startswith(s, "ello wo"));
	EXPECT_FALSE(muggle_str_startswith(s, "ello wor"));
	EXPECT_FALSE(muggle_str_startswith(s, "ello worl"));
	EXPECT_FALSE(muggle_str_startswith(s, "ello world"));
}

TEST(str, endswith)
{
	const char *s = "hello world";

	EXPECT_TRUE(muggle_str_endswith(s, "hello world"));
	EXPECT_TRUE(muggle_str_endswith(s, "ello world"));
	EXPECT_TRUE(muggle_str_endswith(s, "llo world"));
	EXPECT_TRUE(muggle_str_endswith(s, "lo world"));
	EXPECT_TRUE(muggle_str_endswith(s, "o world"));
	EXPECT_TRUE(muggle_str_endswith(s, " world"));
	EXPECT_TRUE(muggle_str_endswith(s, "world"));
	EXPECT_TRUE(muggle_str_endswith(s, "orld"));
	EXPECT_TRUE(muggle_str_endswith(s, "rld"));
	EXPECT_TRUE(muggle_str_endswith(s, "ld"));
	EXPECT_TRUE(muggle_str_endswith(s, "d"));

	EXPECT_FALSE(muggle_str_endswith(s, "hello worl"));
	EXPECT_FALSE(muggle_str_endswith(s, "ello worl"));
	EXPECT_FALSE(muggle_str_endswith(s, "llo worl"));
	EXPECT_FALSE(muggle_str_endswith(s, "lo worl"));
	EXPECT_FALSE(muggle_str_endswith(s, "o worl"));
	EXPECT_FALSE(muggle_str_endswith(s, " worl"));
	EXPECT_FALSE(muggle_str_endswith(s, "worl"));
	EXPECT_FALSE(muggle_str_endswith(s, "orl"));
	EXPECT_FALSE(muggle_str_endswith(s, "rl"));
	EXPECT_FALSE(muggle_str_endswith(s, "l"));
	EXPECT_FALSE(muggle_str_endswith(s, ""));
}

TEST(str, count)
{
	const char *s = "ooooo";

	EXPECT_EQ(muggle_str_count(s, "o", 0, 0), 5);
	EXPECT_EQ(muggle_str_count(s, "oo", 0, 0), 2);
	EXPECT_EQ(muggle_str_count(s, "ooo", 0, 0), 1);
	EXPECT_EQ(muggle_str_count(s, "oooo", 0, 0), 1);
	EXPECT_EQ(muggle_str_count(s, "ooooo", 0, 0), 1);
	EXPECT_EQ(muggle_str_count(s, "oooooo", 0, 0), 0);

	EXPECT_EQ(muggle_str_count(s, "o", 1, 0), 4);
	EXPECT_EQ(muggle_str_count(s, "oo", 1, 0), 2);
	EXPECT_EQ(muggle_str_count(s, "ooo", 1, 0), 1);
	EXPECT_EQ(muggle_str_count(s, "oooo", 1, 0), 1);
	EXPECT_EQ(muggle_str_count(s, "ooooo", 1, 0), 0);
	EXPECT_EQ(muggle_str_count(s, "oooooo", 1, 0), 0);

	EXPECT_EQ(muggle_str_count(s, "o", 2, 0), 3);
	EXPECT_EQ(muggle_str_count(s, "oo", 2, 0), 1);
	EXPECT_EQ(muggle_str_count(s, "ooo", 2, 0), 1);
	EXPECT_EQ(muggle_str_count(s, "oooo", 2, 0), 0);
	EXPECT_EQ(muggle_str_count(s, "ooooo", 2, 0), 0);
	EXPECT_EQ(muggle_str_count(s, "oooooo", 2, 0), 0);

	EXPECT_EQ(muggle_str_count(s, "o", 3, 0), 2);
	EXPECT_EQ(muggle_str_count(s, "oo", 3, 0), 1);
	EXPECT_EQ(muggle_str_count(s, "ooo", 3, 0), 0);
	EXPECT_EQ(muggle_str_count(s, "oooo", 3, 0), 0);
	EXPECT_EQ(muggle_str_count(s, "ooooo", 3, 0), 0);
	EXPECT_EQ(muggle_str_count(s, "oooooo", 3, 0), 0);

	EXPECT_EQ(muggle_str_count(s, "o", 4, 0), 1);
	EXPECT_EQ(muggle_str_count(s, "oo", 4, 0), 0);
	EXPECT_EQ(muggle_str_count(s, "ooo", 4, 0), 0);
	EXPECT_EQ(muggle_str_count(s, "oooo", 4, 0), 0);
	EXPECT_EQ(muggle_str_count(s, "ooooo", 4, 0), 0);
	EXPECT_EQ(muggle_str_count(s, "oooooo", 4, 0), 0);

	EXPECT_EQ(muggle_str_count(s, "o", 5, 0), 0);
	EXPECT_EQ(muggle_str_count(s, "oo", 5, 0), 0);
	EXPECT_EQ(muggle_str_count(s, "ooo", 5, 0), 0);
	EXPECT_EQ(muggle_str_count(s, "oooo", 5, 0), 0);
	EXPECT_EQ(muggle_str_count(s, "ooooo", 5, 0), 0);
	EXPECT_EQ(muggle_str_count(s, "oooooo", 5, 0), 0);

	EXPECT_EQ(muggle_str_count(s, "o", 0, 6), 5);
	EXPECT_EQ(muggle_str_count(s, "oo", 0, 6), 2);
	EXPECT_EQ(muggle_str_count(s, "ooo", 0, 6), 1);
	EXPECT_EQ(muggle_str_count(s, "oooo", 0, 6), 1);
	EXPECT_EQ(muggle_str_count(s, "ooooo", 0, 6), 1);
	EXPECT_EQ(muggle_str_count(s, "oooooo", 0, 6), 0);

	EXPECT_EQ(muggle_str_count(s, "o", 0, 5), 5);
	EXPECT_EQ(muggle_str_count(s, "oo", 0, 5), 2);
	EXPECT_EQ(muggle_str_count(s, "ooo", 0, 5), 1);
	EXPECT_EQ(muggle_str_count(s, "oooo", 0, 5), 1);
	EXPECT_EQ(muggle_str_count(s, "ooooo", 0, 5), 1);
	EXPECT_EQ(muggle_str_count(s, "oooooo", 0, 5), 0);

	EXPECT_EQ(muggle_str_count(s, "o", 0, 4), 4);
	EXPECT_EQ(muggle_str_count(s, "oo", 0, 4), 2);
	EXPECT_EQ(muggle_str_count(s, "ooo", 0, 4), 1);
	EXPECT_EQ(muggle_str_count(s, "oooo", 0, 4), 1);
	EXPECT_EQ(muggle_str_count(s, "ooooo", 0, 4), 0);
	EXPECT_EQ(muggle_str_count(s, "oooooo", 0, 4), 0);

	EXPECT_EQ(muggle_str_count(s, "o", 0, 3), 3);
	EXPECT_EQ(muggle_str_count(s, "oo", 0, 3), 1);
	EXPECT_EQ(muggle_str_count(s, "ooo", 0, 3), 1);
	EXPECT_EQ(muggle_str_count(s, "oooo", 0, 3), 0);
	EXPECT_EQ(muggle_str_count(s, "ooooo", 0, 3), 0);
	EXPECT_EQ(muggle_str_count(s, "oooooo", 0, 3), 0);

	EXPECT_EQ(muggle_str_count(s, "o", 0, 2), 2);
	EXPECT_EQ(muggle_str_count(s, "oo", 0, 2), 1);
	EXPECT_EQ(muggle_str_count(s, "ooo", 0, 2), 0);
	EXPECT_EQ(muggle_str_count(s, "oooo", 0, 2), 0);
	EXPECT_EQ(muggle_str_count(s, "ooooo", 0, 2), 0);
	EXPECT_EQ(muggle_str_count(s, "oooooo", 0, 2), 0);

	EXPECT_EQ(muggle_str_count(s, "o", 0, 1), 1);
	EXPECT_EQ(muggle_str_count(s, "oo", 0, 1), 0);
	EXPECT_EQ(muggle_str_count(s, "ooo", 0, 1), 0);
	EXPECT_EQ(muggle_str_count(s, "oooo", 0, 1), 0);
	EXPECT_EQ(muggle_str_count(s, "ooooo", 0, 1), 0);
	EXPECT_EQ(muggle_str_count(s, "oooooo", 0, 1), 0);
}

TEST(str, find)
{
	const char *s = "hello world";
	EXPECT_EQ(muggle_str_find(s, "w", 0, 0), 6);
	EXPECT_EQ(muggle_str_find(s, "wo", 0, 0), 6);
	EXPECT_EQ(muggle_str_find(s, "wor", 0, 0), 6);
	EXPECT_EQ(muggle_str_find(s, "worl", 0, 0), 6);
	EXPECT_EQ(muggle_str_find(s, "world", 0, 0), 6);

	EXPECT_EQ(muggle_str_find(s, "w", 0, 0), 6);
	EXPECT_EQ(muggle_str_find(s, "w", 1, 0), 6);
	EXPECT_EQ(muggle_str_find(s, "w", 2, 0), 6);
	EXPECT_EQ(muggle_str_find(s, "w", 3, 0), 6);
	EXPECT_EQ(muggle_str_find(s, "w", 4, 0), 6);
	EXPECT_EQ(muggle_str_find(s, "w", 5, 0), 6);
	EXPECT_EQ(muggle_str_find(s, "w", 6, 0), 6);
	EXPECT_EQ(muggle_str_find(s, "w", 7, 0), -1);
	EXPECT_EQ(muggle_str_find(s, "w", 8, 0), -1);
	EXPECT_EQ(muggle_str_find(s, "w", 9, 0), -1);
	EXPECT_EQ(muggle_str_find(s, "w", 10, 0), -1);

	EXPECT_EQ(muggle_str_find(s, "w", 0, 1), -1);
	EXPECT_EQ(muggle_str_find(s, "w", 0, 2), -1);
	EXPECT_EQ(muggle_str_find(s, "w", 0, 3), -1);
	EXPECT_EQ(muggle_str_find(s, "w", 0, 4), -1);
	EXPECT_EQ(muggle_str_find(s, "w", 0, 5), -1);
	EXPECT_EQ(muggle_str_find(s, "w", 0, 6), -1);
	EXPECT_EQ(muggle_str_find(s, "w", 0, 7), 6);
	EXPECT_EQ(muggle_str_find(s, "w", 0, 8), 6);
	EXPECT_EQ(muggle_str_find(s, "w", 0, 9), 6);
}

TEST(str, lstrip)
{
	const char *s1 = " hello";
	const char *s2 = "\tworld";

	EXPECT_EQ(muggle_str_lstrip_idx(s1), 1);
	EXPECT_EQ(muggle_str_lstrip_idx(s2), 1);
}

TEST(str, rstrip)
{
	const char *s1 = "hello ";
	const char *s2 = "world\t";

	EXPECT_EQ(muggle_str_rstrip_idx(s1), 4);
	EXPECT_EQ(muggle_str_rstrip_idx(s2), 4);
}

TEST(str, stoi)
{
	int ret;

	EXPECT_TRUE(muggle_str_toi("5", &ret, 10));
	EXPECT_EQ(ret, 5);
	EXPECT_TRUE(muggle_str_toi(" 5", &ret, 10));
	EXPECT_EQ(ret, 5);
	EXPECT_TRUE(muggle_str_toi("5 ", &ret, 10));
	EXPECT_EQ(ret, 5);
	EXPECT_TRUE(muggle_str_toi(" 5 ", &ret, 10));
	EXPECT_EQ(ret, 5);

	EXPECT_FALSE(muggle_str_toi("5 6", &ret, 10));
	EXPECT_FALSE(muggle_str_toi(" 5 6", &ret, 10));
	EXPECT_FALSE(muggle_str_toi("5 6", &ret, 10));
	EXPECT_FALSE(muggle_str_toi(" 5 6", &ret, 10));
	EXPECT_FALSE(muggle_str_toi("hello", &ret, 10));

	EXPECT_TRUE(muggle_str_toi("-1", &ret, 10));
	EXPECT_EQ(ret, -1);
	EXPECT_TRUE(muggle_str_toi(" -1", &ret, 10));
	EXPECT_EQ(ret, -1);
	EXPECT_TRUE(muggle_str_toi("-1 ", &ret, 10));
	EXPECT_EQ(ret, -1);
	EXPECT_TRUE(muggle_str_toi(" -1 ", &ret, 10));
	EXPECT_EQ(ret, -1);


	EXPECT_TRUE(muggle_str_toi("0xa", &ret, 16));
	EXPECT_EQ(ret, 10);
	EXPECT_TRUE(muggle_str_toi(" 0xa", &ret, 16));
	EXPECT_EQ(ret, 10);
	EXPECT_TRUE(muggle_str_toi("0xa ", &ret, 16));
	EXPECT_EQ(ret, 10);
	EXPECT_TRUE(muggle_str_toi(" 0xa ", &ret, 16));
	EXPECT_EQ(ret, 10);

	EXPECT_FALSE(muggle_str_toi("100000000000000000000000", &ret, 10));
}

TEST(str, stou)
{
	unsigned int ret;

	EXPECT_TRUE(muggle_str_tou("5", &ret, 10));
	EXPECT_EQ(ret, 5);
	EXPECT_TRUE(muggle_str_tou(" 5", &ret, 10));
	EXPECT_EQ(ret, 5);
	EXPECT_TRUE(muggle_str_tou("5 ", &ret, 10));
	EXPECT_EQ(ret, 5);
	EXPECT_TRUE(muggle_str_tou(" 5 ", &ret, 10));
	EXPECT_EQ(ret, 5);

	EXPECT_FALSE(muggle_str_tou("5 6", &ret, 10));
	EXPECT_FALSE(muggle_str_tou(" 5 6", &ret, 10));
	EXPECT_FALSE(muggle_str_tou("5 6", &ret, 10));
	EXPECT_FALSE(muggle_str_tou(" 5 6", &ret, 10));
	EXPECT_FALSE(muggle_str_tou("hello", &ret, 10));

	EXPECT_FALSE(muggle_str_tou("-1", &ret, 10));
	EXPECT_FALSE(muggle_str_tou(" -1", &ret, 10));
	EXPECT_FALSE(muggle_str_tou("-1 ", &ret, 10));
	EXPECT_FALSE(muggle_str_tou(" -1 ", &ret, 10));

	EXPECT_TRUE(muggle_str_tou("0xa", &ret, 16));
	EXPECT_EQ(ret, 10);
	EXPECT_TRUE(muggle_str_tou(" 0xa", &ret, 16));
	EXPECT_EQ(ret, 10);
	EXPECT_TRUE(muggle_str_tou("0xa ", &ret, 16));
	EXPECT_EQ(ret, 10);
	EXPECT_TRUE(muggle_str_tou(" 0xa ", &ret, 16));
	EXPECT_EQ(ret, 10);

	EXPECT_FALSE(muggle_str_tou("100000000000000000000000", &ret, 10));
}

TEST(str, stol)
{
	long ret;

	EXPECT_TRUE(muggle_str_tol("5", &ret, 10));
	EXPECT_EQ(ret, 5);
	EXPECT_TRUE(muggle_str_tol(" 5", &ret, 10));
	EXPECT_EQ(ret, 5);
	EXPECT_TRUE(muggle_str_tol("5 ", &ret, 10));
	EXPECT_EQ(ret, 5);
	EXPECT_TRUE(muggle_str_tol(" 5 ", &ret, 10));
	EXPECT_EQ(ret, 5);

	EXPECT_FALSE(muggle_str_tol("5 6", &ret, 10));
	EXPECT_FALSE(muggle_str_tol(" 5 6", &ret, 10));
	EXPECT_FALSE(muggle_str_tol("5 6", &ret, 10));
	EXPECT_FALSE(muggle_str_tol(" 5 6", &ret, 10));
	EXPECT_FALSE(muggle_str_tol("hello", &ret, 10));

	EXPECT_TRUE(muggle_str_tol("-1", &ret, 10));
	EXPECT_EQ(ret, -1);
	EXPECT_TRUE(muggle_str_tol(" -1", &ret, 10));
	EXPECT_EQ(ret, -1);
	EXPECT_TRUE(muggle_str_tol("-1 ", &ret, 10));
	EXPECT_EQ(ret, -1);
	EXPECT_TRUE(muggle_str_tol(" -1 ", &ret, 10));
	EXPECT_EQ(ret, -1);

	EXPECT_TRUE(muggle_str_tol("0xa", &ret, 16));
	EXPECT_EQ(ret, 10);
	EXPECT_TRUE(muggle_str_tol(" 0xa", &ret, 16));
	EXPECT_EQ(ret, 10);
	EXPECT_TRUE(muggle_str_tol("0xa ", &ret, 16));
	EXPECT_EQ(ret, 10);
	EXPECT_TRUE(muggle_str_tol(" 0xa ", &ret, 16));
	EXPECT_EQ(ret, 10);

	EXPECT_FALSE(muggle_str_tol("100000000000000000000000", &ret, 10));
}

TEST(str, stoul)
{
	unsigned long ret;

	EXPECT_TRUE(muggle_str_toul("5", &ret, 10));
	EXPECT_EQ(ret, 5);
	EXPECT_TRUE(muggle_str_toul(" 5", &ret, 10));
	EXPECT_EQ(ret, 5);
	EXPECT_TRUE(muggle_str_toul("5 ", &ret, 10));
	EXPECT_EQ(ret, 5);
	EXPECT_TRUE(muggle_str_toul(" 5 ", &ret, 10));
	EXPECT_EQ(ret, 5);

	EXPECT_FALSE(muggle_str_toul("5 6", &ret, 10));
	EXPECT_FALSE(muggle_str_toul(" 5 6", &ret, 10));
	EXPECT_FALSE(muggle_str_toul("5 6", &ret, 10));
	EXPECT_FALSE(muggle_str_toul(" 5 6", &ret, 10));
	EXPECT_FALSE(muggle_str_toul("hello", &ret, 10));

	EXPECT_FALSE(muggle_str_toul("-1", &ret, 10));
	EXPECT_FALSE(muggle_str_toul(" -1", &ret, 10));
	EXPECT_FALSE(muggle_str_toul("-1 ", &ret, 10));
	EXPECT_FALSE(muggle_str_toul(" -1 ", &ret, 10));

	EXPECT_TRUE(muggle_str_toul("0xa", &ret, 16));
	EXPECT_EQ(ret, 10);
	EXPECT_TRUE(muggle_str_toul(" 0xa", &ret, 16));
	EXPECT_EQ(ret, 10);
	EXPECT_TRUE(muggle_str_toul("0xa ", &ret, 16));
	EXPECT_EQ(ret, 10);
	EXPECT_TRUE(muggle_str_toul(" 0xa ", &ret, 16));
	EXPECT_EQ(ret, 10);

	EXPECT_FALSE(muggle_str_toul("100000000000000000000000", &ret, 10));
}

TEST(str, stoll)
{
	long long ret;

	EXPECT_TRUE(muggle_str_toll("5", &ret, 10));
	EXPECT_EQ(ret, 5);
	EXPECT_TRUE(muggle_str_toll(" 5", &ret, 10));
	EXPECT_EQ(ret, 5);
	EXPECT_TRUE(muggle_str_toll("5 ", &ret, 10));
	EXPECT_EQ(ret, 5);
	EXPECT_TRUE(muggle_str_toll(" 5 ", &ret, 10));
	EXPECT_EQ(ret, 5);

	EXPECT_FALSE(muggle_str_toll("5 6", &ret, 10));
	EXPECT_FALSE(muggle_str_toll(" 5 6", &ret, 10));
	EXPECT_FALSE(muggle_str_toll("5 6", &ret, 10));
	EXPECT_FALSE(muggle_str_toll(" 5 6", &ret, 10));
	EXPECT_FALSE(muggle_str_toll("hello", &ret, 10));

	EXPECT_TRUE(muggle_str_toll("-1", &ret, 10));
	EXPECT_EQ(ret, -1);
	EXPECT_TRUE(muggle_str_toll(" -1", &ret, 10));
	EXPECT_EQ(ret, -1);
	EXPECT_TRUE(muggle_str_toll("-1 ", &ret, 10));
	EXPECT_EQ(ret, -1);
	EXPECT_TRUE(muggle_str_toll(" -1 ", &ret, 10));
	EXPECT_EQ(ret, -1);

	EXPECT_TRUE(muggle_str_toll("0xa", &ret, 16));
	EXPECT_EQ(ret, 10);
	EXPECT_TRUE(muggle_str_toll(" 0xa", &ret, 16));
	EXPECT_EQ(ret, 10);
	EXPECT_TRUE(muggle_str_toll("0xa ", &ret, 16));
	EXPECT_EQ(ret, 10);
	EXPECT_TRUE(muggle_str_toll(" 0xa ", &ret, 16));
	EXPECT_EQ(ret, 10);

	EXPECT_FALSE(muggle_str_toll("100000000000000000000000", &ret, 10));
}

TEST(str, stoull)
{
	unsigned long long ret;

	EXPECT_TRUE(muggle_str_toull("5", &ret, 10));
	EXPECT_EQ(ret, 5);
	EXPECT_TRUE(muggle_str_toull(" 5", &ret, 10));
	EXPECT_EQ(ret, 5);
	EXPECT_TRUE(muggle_str_toull("5 ", &ret, 10));
	EXPECT_EQ(ret, 5);
	EXPECT_TRUE(muggle_str_toull(" 5 ", &ret, 10));
	EXPECT_EQ(ret, 5);

	EXPECT_FALSE(muggle_str_toull("5 6", &ret, 10));
	EXPECT_FALSE(muggle_str_toull(" 5 6", &ret, 10));
	EXPECT_FALSE(muggle_str_toull("5 6", &ret, 10));
	EXPECT_FALSE(muggle_str_toull(" 5 6", &ret, 10));
	EXPECT_FALSE(muggle_str_toull("hello", &ret, 10));

	EXPECT_FALSE(muggle_str_toull("-1", &ret, 10));
	EXPECT_FALSE(muggle_str_toull(" -1", &ret, 10));
	EXPECT_FALSE(muggle_str_toull("-1 ", &ret, 10));
	EXPECT_FALSE(muggle_str_toull(" -1 ", &ret, 10));

	EXPECT_TRUE(muggle_str_toull("0xa", &ret, 16));
	EXPECT_EQ(ret, 10);
	EXPECT_TRUE(muggle_str_toull(" 0xa", &ret, 16));
	EXPECT_EQ(ret, 10);
	EXPECT_TRUE(muggle_str_toull("0xa ", &ret, 16));
	EXPECT_EQ(ret, 10);
	EXPECT_TRUE(muggle_str_toull(" 0xa ", &ret, 16));
	EXPECT_EQ(ret, 10);

	EXPECT_FALSE(muggle_str_toull("100000000000000000000000", &ret, 10));
}

TEST(str, stof)
{
	float ret;

	EXPECT_TRUE(muggle_str_tof("11.0", &ret));
	EXPECT_FLOAT_EQ(ret, 11.0);
	EXPECT_TRUE(muggle_str_tof(" 11.0", &ret));
	EXPECT_FLOAT_EQ(ret, 11.0);
	EXPECT_TRUE(muggle_str_tof("11.0 ", &ret));
	EXPECT_FLOAT_EQ(ret, 11.0);
	EXPECT_TRUE(muggle_str_tof(" 11.0 ", &ret));
	EXPECT_FLOAT_EQ(ret, 11.0);

	EXPECT_FALSE(muggle_str_tof("11.0 5", &ret));

	EXPECT_TRUE(muggle_str_tof("nan", &ret));
	EXPECT_TRUE(std::isnan(ret));
	EXPECT_FALSE(muggle_str_tof("10e300", &ret));
	EXPECT_FALSE(muggle_str_tof("hello", &ret));
}

TEST(str, stod)
{
	double ret;

	EXPECT_TRUE(muggle_str_tod("11.0", &ret));
	EXPECT_DOUBLE_EQ(ret, 11.0);
	EXPECT_TRUE(muggle_str_tod(" 11.0", &ret));
	EXPECT_DOUBLE_EQ(ret, 11.0);
	EXPECT_TRUE(muggle_str_tod("11.0 ", &ret));
	EXPECT_DOUBLE_EQ(ret, 11.0);
	EXPECT_TRUE(muggle_str_tod(" 11.0 ", &ret));
	EXPECT_DOUBLE_EQ(ret, 11.0);

	EXPECT_FALSE(muggle_str_tod("11.0 5", &ret));

	EXPECT_TRUE(muggle_str_tod("nan", &ret));
	EXPECT_TRUE(std::isnan(ret));
	EXPECT_FALSE(muggle_str_tod("10e10000", &ret));
	EXPECT_FALSE(muggle_str_tod("hello", &ret));
}

TEST(str, stold)
{
	long double ret;

	EXPECT_TRUE(muggle_str_told("11.0", &ret));
	EXPECT_DOUBLE_EQ(ret, 11.0);
	EXPECT_TRUE(muggle_str_told(" 11.0", &ret));
	EXPECT_DOUBLE_EQ(ret, 11.0);
	EXPECT_TRUE(muggle_str_told("11.0 ", &ret));
	EXPECT_DOUBLE_EQ(ret, 11.0);
	EXPECT_TRUE(muggle_str_told(" 11.0 ", &ret));
	EXPECT_DOUBLE_EQ(ret, 11.0);

	EXPECT_FALSE(muggle_str_told("11.0 5", &ret));

	EXPECT_TRUE(muggle_str_told("nan", &ret));
	EXPECT_TRUE(std::isnan(ret));
	EXPECT_FALSE(muggle_str_told("10e10000", &ret));
	EXPECT_FALSE(muggle_str_told("hello", &ret));
}

