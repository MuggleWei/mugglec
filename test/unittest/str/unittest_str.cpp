#include <time.h>
#include <stddef.h>
#include <limits.h>
#include <float.h>
#include "gtest/gtest.h"
#include "muggle/base/base.h"

const char *str1 = "\
	  First line(1).\n\
\tSecond Line(2).\r\n\
Third Line(3).\n		\
	";
const char *words1[] = {
	"First", "line(1).",
	"Second", "Line(2).",
	"Third", "Line(3)."
};
const char *lines1[] = {
	"First line(1).",
	"Second Line(2).",
	"Third Line(3)."
};

TEST(Str, Move)
{	
	int idx = 0;
	int cnt = (int)(sizeof(words1) / sizeof(words1[0]));
	const char *p = str1, *q = nullptr;

	// words1
	MUGGLE_SKIP_BLANK(p);
	while (*p != '\0')
	{
		ASSERT_TRUE(strncmp(p, words1[idx], strlen(words1[idx])) == 0);
		MUGGLE_SKIP_TO_NEXT_WORD(p);
		++idx;
	}
	ASSERT_EQ(idx, cnt);

	// lines1
	idx = 0;
	cnt = (int)(sizeof(lines1) / sizeof(lines1[0]));
	p = str1;
	while (*p != '\0')
	{
		MUGGLE_SKIP_BLANK(p);
		q = p;
		MUGGLE_SKIP_TO_LINE_END(q);
		size_t l = q - p;
		ASSERT_EQ(l, strlen(lines1[idx]));
		MUGGLE_SKIP_TO_NEXT_LINE(p);
		++idx;
	}
	ASSERT_EQ(idx, cnt);
}

TEST(Str, Operator)
{
	// range allocate
	const char *p = nullptr, *q = nullptr;

	p = str1;
	MUGGLE_SKIP_BLANK(p);
	q = str1;
	while (*q != '\0')
	{
		++q;
	}
	--q;

	char *str = StrAllocByDiff(p, q);
	ASSERT_TRUE(strcmp(str, p) == 0);	

	// split line into words
	constexpr int max_word_num = 16;
	char *word_array[max_word_num];
	char **w = word_array;
	int cnt = 0, sum = 0;
	char *p_line = str;
	do
	{
		MUGGLE_SKIP_BLANK(p_line);
		p_line = StrSplitLineToWords(p_line, w + sum, &cnt, max_word_num - sum);
		sum += cnt;
		MUGGLE_SKIP_BLANK(p_line);
	} while (*p_line != '\0');

	int num = (int)(sizeof(words1) / sizeof(words1[0]));
	ASSERT_EQ(num, sum);

	for (int i = 0; i < sum; ++i)
	{
		ASSERT_TRUE(strcmp(words1[i], word_array[i]) == 0);
	}

	free(str);

	// sub string
	ASSERT_TRUE(StrStartsWith("Hello world", "H"));
	ASSERT_TRUE(StrStartsWith("Hello world", "He"));
	ASSERT_TRUE(StrStartsWith("Hello world", "Hel"));
	ASSERT_TRUE(StrStartsWith("Hello world", "Hell"));
	ASSERT_TRUE(StrStartsWith("Hello world", "Hello"));
	ASSERT_TRUE(StrStartsWith("Hello world", "Hello "));
	ASSERT_TRUE(StrStartsWith("Hello world", "Hello w"));
	ASSERT_TRUE(StrStartsWith("Hello world", "Hello wo"));
	ASSERT_TRUE(StrStartsWith("Hello world", "Hello wor"));
	ASSERT_TRUE(StrStartsWith("Hello world", "Hello worl"));
	ASSERT_TRUE(StrStartsWith("Hello world", "Hello world"));

	ASSERT_TRUE(StrEndsWith("Hello world", "d"));
	ASSERT_TRUE(StrEndsWith("Hello world", "ld"));
	ASSERT_TRUE(StrEndsWith("Hello world", "rld"));
	ASSERT_TRUE(StrEndsWith("Hello world", "orld"));
	ASSERT_TRUE(StrEndsWith("Hello world", "world"));
	ASSERT_TRUE(StrEndsWith("Hello world", " world"));
	ASSERT_TRUE(StrEndsWith("Hello world", "o world"));
	ASSERT_TRUE(StrEndsWith("Hello world", "lo world"));
	ASSERT_TRUE(StrEndsWith("Hello world", "llo world"));
	ASSERT_TRUE(StrEndsWith("Hello world", "ello world"));
	ASSERT_TRUE(StrEndsWith("Hello world", "Hello world"));

}

TEST(Str, Toi)
{
	constexpr size_t buf_size = 128;
	char buf[buf_size];
	int val;

	EXPECT_TRUE(StrToi("1", &val, 0));
	EXPECT_EQ(val, 1);
	EXPECT_TRUE(StrToi("22", &val, 0));
	EXPECT_EQ(val, 22);
	EXPECT_TRUE(StrToi("0x10", &val, 0));
	EXPECT_EQ(val, 0x10);
	EXPECT_TRUE(StrToi("0xA6", &val, 0));
	EXPECT_EQ(val, 0xA6);
	EXPECT_TRUE(StrToi("010", &val, 0));
	EXPECT_EQ(val, 010);
	EXPECT_TRUE(StrToi("-15", &val, 0));
	EXPECT_EQ(val, -15);
	snprintf(buf, buf_size, "%d", INT_MAX);
	EXPECT_TRUE(StrToi(buf, &val, 0));
	EXPECT_EQ(val, INT_MAX);
	snprintf(buf, buf_size, "%d", INT_MIN);
	EXPECT_TRUE(StrToi(buf, &val, 0));
	EXPECT_EQ(val, INT_MIN);

	EXPECT_FALSE(StrToi("", &val, 0));
	EXPECT_FALSE(StrToi("1L", &val, 0));
	EXPECT_FALSE(StrToi("22l", &val, 0));
	EXPECT_FALSE(StrToi("jdkf", &val, 0));
	EXPECT_FALSE(StrToi("1.1f", &val, 0));
	snprintf(buf, buf_size, "1%d", INT_MAX);
	EXPECT_FALSE(StrToi(buf, &val, 0));
	snprintf(buf, buf_size, "%d1", INT_MIN);
	EXPECT_FALSE(StrToi(buf, &val, 0));
}
TEST(Str, Tou)
{
	constexpr size_t buf_size = 128;
	char buf[buf_size];
	unsigned int val;

	EXPECT_TRUE(StrToui("1", &val, 0));
	EXPECT_EQ(val, 1u);
	EXPECT_TRUE(StrToui("22", &val, 0));
	EXPECT_EQ(val, 22u);
	EXPECT_TRUE(StrToui("0x10", &val, 0));
	EXPECT_EQ(val, (unsigned int)0x10);
	EXPECT_TRUE(StrToui("0xA6", &val, 0));
	EXPECT_EQ(val, (unsigned int)0xA6);
	EXPECT_TRUE(StrToui("010", &val, 0));
	EXPECT_EQ(val, (unsigned int)010);
	// This is not sure, dependent the size of ULONG_MAX and UINT_MAX
	// EXPECT_TRUE(StrToui("-15", &val, 0));
	// EXPECT_EQ(val, (unsigned int)-15);
	snprintf(buf, buf_size, "%u", UINT_MAX);
	EXPECT_TRUE(StrToui(buf, &val, 0));
	EXPECT_EQ(val, UINT_MAX);

	EXPECT_FALSE(StrToui("", &val, 0));
	EXPECT_FALSE(StrToui("1L", &val, 0));
	EXPECT_FALSE(StrToui("22l", &val, 0));
	EXPECT_FALSE(StrToui("jdkf", &val, 0));
	EXPECT_FALSE(StrToui("1.1f", &val, 0));
	snprintf(buf, buf_size, "1%u", UINT_MAX);
}
TEST(Str, Tol)
{
	constexpr size_t buf_size = 128;
	char buf[buf_size];
	long val;

	EXPECT_TRUE(StrTol("1", &val, 0));
	EXPECT_EQ(val, 1);
	EXPECT_TRUE(StrTol("22", &val, 0));
	EXPECT_EQ(val, 22);
	EXPECT_TRUE(StrTol("0x10", &val, 0));
	EXPECT_EQ(val, 0x10);
	EXPECT_TRUE(StrTol("0xA6", &val, 0));
	EXPECT_EQ(val, 0xA6);
	EXPECT_TRUE(StrTol("010", &val, 0));
	EXPECT_EQ(val, 010);
	EXPECT_TRUE(StrTol("-15", &val, 0));
	EXPECT_EQ(val, -15);
	snprintf(buf, buf_size, "%ld", LONG_MAX);
	EXPECT_TRUE(StrTol(buf, &val, 0));
	EXPECT_EQ(val, LONG_MAX);
	snprintf(buf, buf_size, "%ld", LONG_MIN);
	EXPECT_TRUE(StrTol(buf, &val, 0));
	EXPECT_EQ(val, LONG_MIN);

	EXPECT_FALSE(StrTol("", &val, 0));
	EXPECT_FALSE(StrTol("1L", &val, 0));
	EXPECT_FALSE(StrTol("22l", &val, 0));
	EXPECT_FALSE(StrTol("jdkf", &val, 0));
	EXPECT_FALSE(StrTol("1.1f", &val, 0));
	snprintf(buf, buf_size, "1%ld", LONG_MAX);
	EXPECT_FALSE(StrTol(buf, &val, 0));
	snprintf(buf, buf_size, "%ld1", LONG_MIN);
	EXPECT_FALSE(StrTol(buf, &val, 0));
}
TEST(Str, Toul)
{
	constexpr size_t buf_size = 128;
	char buf[buf_size];
	unsigned long val;

	EXPECT_TRUE(StrToul("1", &val, 0));
	EXPECT_EQ(val, 1u);
	EXPECT_TRUE(StrToul("22", &val, 0));
	EXPECT_EQ(val, 22u);
	EXPECT_TRUE(StrToul("0x10", &val, 0));
	EXPECT_EQ(val, (unsigned long)0x10);
	EXPECT_TRUE(StrToul("0xA6", &val, 0));
	EXPECT_EQ(val, (unsigned long)0xA6);
	EXPECT_TRUE(StrToul("010", &val, 0));
	EXPECT_EQ(val, (unsigned long)010);
	EXPECT_TRUE(StrToul("-15", &val, 0));
	EXPECT_EQ(val, (unsigned long)-15);
	snprintf(buf, buf_size, "%lu", ULONG_MAX);
	EXPECT_TRUE(StrToul(buf, &val, 0));
	EXPECT_EQ(val, ULONG_MAX);

	EXPECT_FALSE(StrToul("", &val, 0));
	EXPECT_FALSE(StrToul("1L", &val, 0));
	EXPECT_FALSE(StrToul("22l", &val, 0));
	EXPECT_FALSE(StrToul("jdkf", &val, 0));
	EXPECT_FALSE(StrToul("1.1f", &val, 0));
	snprintf(buf, buf_size, "1%lu", ULONG_MAX);
}
TEST(Str, Toll)
{
	constexpr size_t buf_size = 128;
	char buf[buf_size];
	long long val;

	EXPECT_TRUE(StrToll("1", &val, 0));
	EXPECT_EQ(val, (long long)1);
	EXPECT_TRUE(StrToll("22", &val, 0));
	EXPECT_EQ(val, (long long)22);
	EXPECT_TRUE(StrToll("0x10", &val, 0));
	EXPECT_EQ(val, (long long)0x10);
	EXPECT_TRUE(StrToll("0xA6", &val, 0));
	EXPECT_EQ(val, (long long)0xA6);
	EXPECT_TRUE(StrToll("010", &val, 0));
	EXPECT_EQ(val, (long long)010);
	EXPECT_TRUE(StrToll("-15", &val, 0));
	EXPECT_EQ(val, (long long)-15);
	snprintf(buf, buf_size, "%lld", LLONG_MAX);
	EXPECT_TRUE(StrToll(buf, &val, 0));
	EXPECT_EQ(val, LLONG_MAX);
	snprintf(buf, buf_size, "%lld", LLONG_MIN);
	EXPECT_TRUE(StrToll(buf, &val, 0));
	EXPECT_EQ(val, LLONG_MIN);

	EXPECT_FALSE(StrToll("", &val, 0));
	EXPECT_FALSE(StrToll("1L", &val, 0));
	EXPECT_FALSE(StrToll("22l", &val, 0));
	EXPECT_FALSE(StrToll("jdkf", &val, 0));
	EXPECT_FALSE(StrToll("1.1f", &val, 0));
	snprintf(buf, buf_size, "1%lld", LLONG_MAX);
	EXPECT_FALSE(StrToll(buf, &val, 0));
	snprintf(buf, buf_size, "%lld1", LLONG_MIN);
	EXPECT_FALSE(StrToll(buf, &val, 0));
}
TEST(Str, Toull)
{
	constexpr size_t buf_size = 128;
	char buf[buf_size];
	unsigned long long val;

	EXPECT_TRUE(StrToull("1", &val, 0));
	EXPECT_EQ(val, (unsigned long long)1);
	EXPECT_TRUE(StrToull("22", &val, 0));
	EXPECT_EQ(val, (unsigned long long)22);
	EXPECT_TRUE(StrToull("0x10", &val, 0));
	EXPECT_EQ(val, (unsigned long long)0x10);
	EXPECT_TRUE(StrToull("0xA6", &val, 0));
	EXPECT_EQ(val, (unsigned long long)0xA6);
	EXPECT_TRUE(StrToull("010", &val, 0));
	EXPECT_EQ(val, (unsigned long long)010);
	EXPECT_TRUE(StrToull("-15", &val, 0));
	EXPECT_EQ(val, (unsigned long long)-15);
	snprintf(buf, buf_size, "%llu", ULLONG_MAX);
	EXPECT_TRUE(StrToull(buf, &val, 0));
	EXPECT_EQ(val, ULLONG_MAX);

	EXPECT_FALSE(StrToull("", &val, 0));
	EXPECT_FALSE(StrToull("1L", &val, 0));
	EXPECT_FALSE(StrToull("22l", &val, 0));
	EXPECT_FALSE(StrToull("jdkf", &val, 0));
	EXPECT_FALSE(StrToull("1.1f", &val, 0));
	snprintf(buf, buf_size, "1%llu", ULLONG_MAX);
}
TEST(Str, Tof)
{
	constexpr size_t buf_size = 128;
	char buf[buf_size];
	float val;

	EXPECT_TRUE(StrTof("1.0", &val));
	EXPECT_FLOAT_EQ(val, 1.0f);
	EXPECT_TRUE(StrTof("20.0", &val));
	EXPECT_FLOAT_EQ(val, 20.0f);
	EXPECT_TRUE(StrTof("-15.0", &val));
	EXPECT_FLOAT_EQ(val, -15.0f);
	snprintf(buf, buf_size, "%.10e", FLT_MAX);
	EXPECT_TRUE(StrTof(buf, &val));
	EXPECT_FLOAT_EQ(val, FLT_MAX);
	snprintf(buf, buf_size, "%.10e", FLT_MIN);
	EXPECT_TRUE(StrTof(buf, &val));
	EXPECT_NEAR(val, FLT_MIN, FLT_EPSILON);

	EXPECT_FALSE(StrTof("", &val));
	EXPECT_FALSE(StrTof("1f", &val));
	EXPECT_FALSE(StrTof("1.1.1", &val));
	snprintf(buf, buf_size, "1%.10e", FLT_MAX);
	EXPECT_FALSE(StrTof(buf, &val));
}
TEST(Str, Tod)
{
	constexpr size_t buf_size = 128;
	char buf[buf_size];
	double val;

	EXPECT_TRUE(StrTod("1.0", &val));
	EXPECT_DOUBLE_EQ(val, 1.0);
	EXPECT_TRUE(StrTod("20.0", &val));
	EXPECT_DOUBLE_EQ(val, 20.0);
	EXPECT_TRUE(StrTod("-15.0", &val));
	EXPECT_DOUBLE_EQ(val, -15.0);
	// snprintf(buf, buf_size, "%.10e", DBL_MAX);
	// EXPECT_TRUE(StrTod(buf, &val));
	// EXPECT_DOUBLE_EQ(val, DBL_MAX);
	snprintf(buf, buf_size, "%.10e", DBL_MIN);
	EXPECT_TRUE(StrTod(buf, &val));
	EXPECT_NEAR(val, FLT_MIN, DBL_EPSILON);

	EXPECT_FALSE(StrTod("", &val));
	EXPECT_FALSE(StrTod("1f", &val));
	EXPECT_FALSE(StrTod("1.1.1", &val));
	snprintf(buf, buf_size, "1%.10e", DBL_MAX);
	EXPECT_FALSE(StrTod(buf, &val));
}