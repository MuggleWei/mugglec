#include <time.h>
#include <stddef.h>
#include <limits.h>
#include <float.h>
#include "gtest/gtest.h"
#include "muggle/base_c/base_c.h"

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

	char *str = MuggleStrAllocByDiff(p, q);
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
		p_line = MuggleStrSplitLineToWords(p_line, w + sum, &cnt, max_word_num - sum);
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
	ASSERT_TRUE(MuggleStrStartsWith("Hello world", "H"));
	ASSERT_TRUE(MuggleStrStartsWith("Hello world", "He"));
	ASSERT_TRUE(MuggleStrStartsWith("Hello world", "Hel"));
	ASSERT_TRUE(MuggleStrStartsWith("Hello world", "Hell"));
	ASSERT_TRUE(MuggleStrStartsWith("Hello world", "Hello"));
	ASSERT_TRUE(MuggleStrStartsWith("Hello world", "Hello "));
	ASSERT_TRUE(MuggleStrStartsWith("Hello world", "Hello w"));
	ASSERT_TRUE(MuggleStrStartsWith("Hello world", "Hello wo"));
	ASSERT_TRUE(MuggleStrStartsWith("Hello world", "Hello wor"));
	ASSERT_TRUE(MuggleStrStartsWith("Hello world", "Hello worl"));
	ASSERT_TRUE(MuggleStrStartsWith("Hello world", "Hello world"));

	ASSERT_TRUE(MuggleStrEndsWith("Hello world", "d"));
	ASSERT_TRUE(MuggleStrEndsWith("Hello world", "ld"));
	ASSERT_TRUE(MuggleStrEndsWith("Hello world", "rld"));
	ASSERT_TRUE(MuggleStrEndsWith("Hello world", "orld"));
	ASSERT_TRUE(MuggleStrEndsWith("Hello world", "world"));
	ASSERT_TRUE(MuggleStrEndsWith("Hello world", " world"));
	ASSERT_TRUE(MuggleStrEndsWith("Hello world", "o world"));
	ASSERT_TRUE(MuggleStrEndsWith("Hello world", "lo world"));
	ASSERT_TRUE(MuggleStrEndsWith("Hello world", "llo world"));
	ASSERT_TRUE(MuggleStrEndsWith("Hello world", "ello world"));
	ASSERT_TRUE(MuggleStrEndsWith("Hello world", "Hello world"));

}

TEST(Str, Toi)
{
	constexpr size_t buf_size = 128;
	char buf[buf_size];
	int val;

	EXPECT_TRUE(MuggleStrToi("1", &val, 0));
	EXPECT_EQ(val, 1);
	EXPECT_TRUE(MuggleStrToi("22", &val, 0));
	EXPECT_EQ(val, 22);
	EXPECT_TRUE(MuggleStrToi("0x10", &val, 0));
	EXPECT_EQ(val, 0x10);
	EXPECT_TRUE(MuggleStrToi("0xA6", &val, 0));
	EXPECT_EQ(val, 0xA6);
	EXPECT_TRUE(MuggleStrToi("010", &val, 0));
	EXPECT_EQ(val, 010);
	EXPECT_TRUE(MuggleStrToi("-15", &val, 0));
	EXPECT_EQ(val, -15);
	snprintf(buf, buf_size, "%d", INT_MAX);
	EXPECT_TRUE(MuggleStrToi(buf, &val, 0));
	EXPECT_EQ(val, INT_MAX);
	snprintf(buf, buf_size, "%d", INT_MIN);
	EXPECT_TRUE(MuggleStrToi(buf, &val, 0));
	EXPECT_EQ(val, INT_MIN);

	EXPECT_FALSE(MuggleStrToi(NULL, &val, 0));
	EXPECT_FALSE(MuggleStrToi("", NULL, 0));
	EXPECT_FALSE(MuggleStrToi("", &val, 0));
	EXPECT_FALSE(MuggleStrToi("1L", &val, 0));
	EXPECT_FALSE(MuggleStrToi("22l", &val, 0));
	EXPECT_FALSE(MuggleStrToi("jdkf", &val, 0));
	EXPECT_FALSE(MuggleStrToi("1.1f", &val, 0));
	snprintf(buf, buf_size, "1%d", INT_MAX);
	EXPECT_FALSE(MuggleStrToi(buf, &val, 0));
	snprintf(buf, buf_size, "%d1", INT_MIN);
	EXPECT_FALSE(MuggleStrToi(buf, &val, 0));
}
TEST(Str, Tou)
{
	constexpr size_t buf_size = 128;
	char buf[buf_size];
	unsigned int val;

	EXPECT_TRUE(MuggleStrToui("1", &val, 0));
	EXPECT_EQ(val, 1u);
	EXPECT_TRUE(MuggleStrToui("22", &val, 0));
	EXPECT_EQ(val, 22u);
	EXPECT_TRUE(MuggleStrToui("0x10", &val, 0));
	EXPECT_EQ(val, (unsigned int)0x10);
	EXPECT_TRUE(MuggleStrToui("0xA6", &val, 0));
	EXPECT_EQ(val, (unsigned int)0xA6);
	EXPECT_TRUE(MuggleStrToui("010", &val, 0));
	EXPECT_EQ(val, (unsigned int)010);
	// This is not sure, dependent the size of ULONG_MAX and UINT_MAX
	// EXPECT_TRUE(MuggleStrToui("-15", &val, 0));
	// EXPECT_EQ(val, (unsigned int)-15);
	snprintf(buf, buf_size, "%u", UINT_MAX);
	EXPECT_TRUE(MuggleStrToui(buf, &val, 0));
	EXPECT_EQ(val, UINT_MAX);

	EXPECT_FALSE(MuggleStrToui(NULL, &val, 0));
	EXPECT_FALSE(MuggleStrToui("", NULL, 0));
	EXPECT_FALSE(MuggleStrToui("", &val, 0));
	EXPECT_FALSE(MuggleStrToui("1L", &val, 0));
	EXPECT_FALSE(MuggleStrToui("22l", &val, 0));
	EXPECT_FALSE(MuggleStrToui("jdkf", &val, 0));
	EXPECT_FALSE(MuggleStrToui("1.1f", &val, 0));
	snprintf(buf, buf_size, "1%u", UINT_MAX);
}
TEST(Str, Tol)
{
	constexpr size_t buf_size = 128;
	char buf[buf_size];
	long val;

	EXPECT_TRUE(MuggleStrTol("1", &val, 0));
	EXPECT_EQ(val, 1);
	EXPECT_TRUE(MuggleStrTol("22", &val, 0));
	EXPECT_EQ(val, 22);
	EXPECT_TRUE(MuggleStrTol("0x10", &val, 0));
	EXPECT_EQ(val, 0x10);
	EXPECT_TRUE(MuggleStrTol("0xA6", &val, 0));
	EXPECT_EQ(val, 0xA6);
	EXPECT_TRUE(MuggleStrTol("010", &val, 0));
	EXPECT_EQ(val, 010);
	EXPECT_TRUE(MuggleStrTol("-15", &val, 0));
	EXPECT_EQ(val, -15);
	snprintf(buf, buf_size, "%ld", LONG_MAX);
	EXPECT_TRUE(MuggleStrTol(buf, &val, 0));
	EXPECT_EQ(val, LONG_MAX);
	snprintf(buf, buf_size, "%ld", LONG_MIN);
	EXPECT_TRUE(MuggleStrTol(buf, &val, 0));
	EXPECT_EQ(val, LONG_MIN);

	EXPECT_FALSE(MuggleStrTol(NULL, &val, 0));
	EXPECT_FALSE(MuggleStrTol("", NULL, 0));
	EXPECT_FALSE(MuggleStrTol("", &val, 0));
	EXPECT_FALSE(MuggleStrTol("1L", &val, 0));
	EXPECT_FALSE(MuggleStrTol("22l", &val, 0));
	EXPECT_FALSE(MuggleStrTol("jdkf", &val, 0));
	EXPECT_FALSE(MuggleStrTol("1.1f", &val, 0));
	snprintf(buf, buf_size, "1%ld", LONG_MAX);
	EXPECT_FALSE(MuggleStrTol(buf, &val, 0));
	snprintf(buf, buf_size, "%ld1", LONG_MIN);
	EXPECT_FALSE(MuggleStrTol(buf, &val, 0));
}
TEST(Str, Toul)
{
	constexpr size_t buf_size = 128;
	char buf[buf_size];
	unsigned long val;

	EXPECT_TRUE(MuggleStrToul("1", &val, 0));
	EXPECT_EQ(val, 1u);
	EXPECT_TRUE(MuggleStrToul("22", &val, 0));
	EXPECT_EQ(val, 22u);
	EXPECT_TRUE(MuggleStrToul("0x10", &val, 0));
	EXPECT_EQ(val, (unsigned long)0x10);
	EXPECT_TRUE(MuggleStrToul("0xA6", &val, 0));
	EXPECT_EQ(val, (unsigned long)0xA6);
	EXPECT_TRUE(MuggleStrToul("010", &val, 0));
	EXPECT_EQ(val, (unsigned long)010);
	EXPECT_TRUE(MuggleStrToul("-15", &val, 0));
	EXPECT_EQ(val, (unsigned long)-15);
	snprintf(buf, buf_size, "%lu", ULONG_MAX);
	EXPECT_TRUE(MuggleStrToul(buf, &val, 0));
	EXPECT_EQ(val, ULONG_MAX);

	EXPECT_FALSE(MuggleStrToul(NULL, &val, 0));
	EXPECT_FALSE(MuggleStrToul("", NULL, 0));
	EXPECT_FALSE(MuggleStrToul("", &val, 0));
	EXPECT_FALSE(MuggleStrToul("1L", &val, 0));
	EXPECT_FALSE(MuggleStrToul("22l", &val, 0));
	EXPECT_FALSE(MuggleStrToul("jdkf", &val, 0));
	EXPECT_FALSE(MuggleStrToul("1.1f", &val, 0));
	snprintf(buf, buf_size, "1%lu", ULONG_MAX);
}
TEST(Str, Toll)
{
	constexpr size_t buf_size = 128;
	char buf[buf_size];
	long long val;

	EXPECT_TRUE(MuggleStrToll("1", &val, 0));
	EXPECT_EQ(val, (long long)1);
	EXPECT_TRUE(MuggleStrToll("22", &val, 0));
	EXPECT_EQ(val, (long long)22);
	EXPECT_TRUE(MuggleStrToll("0x10", &val, 0));
	EXPECT_EQ(val, (long long)0x10);
	EXPECT_TRUE(MuggleStrToll("0xA6", &val, 0));
	EXPECT_EQ(val, (long long)0xA6);
	EXPECT_TRUE(MuggleStrToll("010", &val, 0));
	EXPECT_EQ(val, (long long)010);
	EXPECT_TRUE(MuggleStrToll("-15", &val, 0));
	EXPECT_EQ(val, (long long)-15);
	snprintf(buf, buf_size, "%lld", LLONG_MAX);
	EXPECT_TRUE(MuggleStrToll(buf, &val, 0));
	EXPECT_EQ(val, LLONG_MAX);
	snprintf(buf, buf_size, "%lld", LLONG_MIN);
	EXPECT_TRUE(MuggleStrToll(buf, &val, 0));
	EXPECT_EQ(val, LLONG_MIN);

	EXPECT_FALSE(MuggleStrToll(NULL, &val, 0));
	EXPECT_FALSE(MuggleStrToll("", NULL, 0));
	EXPECT_FALSE(MuggleStrToll("", &val, 0));
	EXPECT_FALSE(MuggleStrToll("1L", &val, 0));
	EXPECT_FALSE(MuggleStrToll("22l", &val, 0));
	EXPECT_FALSE(MuggleStrToll("jdkf", &val, 0));
	EXPECT_FALSE(MuggleStrToll("1.1f", &val, 0));
	snprintf(buf, buf_size, "1%lld", LLONG_MAX);
	EXPECT_FALSE(MuggleStrToll(buf, &val, 0));
	snprintf(buf, buf_size, "%lld1", LLONG_MIN);
	EXPECT_FALSE(MuggleStrToll(buf, &val, 0));
}
TEST(Str, Toull)
{
	constexpr size_t buf_size = 128;
	char buf[buf_size];
	unsigned long long val;

	EXPECT_TRUE(MuggleStrToull("1", &val, 0));
	EXPECT_EQ(val, (unsigned long long)1);
	EXPECT_TRUE(MuggleStrToull("22", &val, 0));
	EXPECT_EQ(val, (unsigned long long)22);
	EXPECT_TRUE(MuggleStrToull("0x10", &val, 0));
	EXPECT_EQ(val, (unsigned long long)0x10);
	EXPECT_TRUE(MuggleStrToull("0xA6", &val, 0));
	EXPECT_EQ(val, (unsigned long long)0xA6);
	EXPECT_TRUE(MuggleStrToull("010", &val, 0));
	EXPECT_EQ(val, (unsigned long long)010);
	EXPECT_TRUE(MuggleStrToull("-15", &val, 0));
	EXPECT_EQ(val, (unsigned long long)-15);
	snprintf(buf, buf_size, "%llu", ULLONG_MAX);
	EXPECT_TRUE(MuggleStrToull(buf, &val, 0));
	EXPECT_EQ(val, ULLONG_MAX);

	EXPECT_FALSE(MuggleStrToull(NULL, &val, 0));
	EXPECT_FALSE(MuggleStrToull("", NULL, 0));
	EXPECT_FALSE(MuggleStrToull("", &val, 0));
	EXPECT_FALSE(MuggleStrToull("1L", &val, 0));
	EXPECT_FALSE(MuggleStrToull("22l", &val, 0));
	EXPECT_FALSE(MuggleStrToull("jdkf", &val, 0));
	EXPECT_FALSE(MuggleStrToull("1.1f", &val, 0));
	snprintf(buf, buf_size, "1%llu", ULLONG_MAX);
}
TEST(Str, Tof)
{
	constexpr size_t buf_size = 128;
	char buf[buf_size];
	float val;

	EXPECT_TRUE(MuggleStrTof("1.0", &val));
	EXPECT_FLOAT_EQ(val, 1.0f);
	EXPECT_TRUE(MuggleStrTof("20.0", &val));
	EXPECT_FLOAT_EQ(val, 20.0f);
	EXPECT_TRUE(MuggleStrTof("-15.0", &val));
	EXPECT_FLOAT_EQ(val, -15.0f);
	snprintf(buf, buf_size, "%.10e", FLT_MAX);
	EXPECT_TRUE(MuggleStrTof(buf, &val));
	EXPECT_FLOAT_EQ(val, FLT_MAX);
	snprintf(buf, buf_size, "%.10e", FLT_MIN);
	EXPECT_TRUE(MuggleStrTof(buf, &val));
	EXPECT_NEAR(val, FLT_MIN, FLT_EPSILON);

	EXPECT_FALSE(MuggleStrTof(NULL, &val));
	EXPECT_FALSE(MuggleStrTof("", NULL));
	EXPECT_FALSE(MuggleStrTof("", &val));
	EXPECT_FALSE(MuggleStrTof("1f", &val));
	EXPECT_FALSE(MuggleStrTof("1.1.1", &val));
	snprintf(buf, buf_size, "1%.10e", FLT_MAX);
	EXPECT_FALSE(MuggleStrTof(buf, &val));
}
TEST(Str, Tod)
{
	constexpr size_t buf_size = 128;
	char buf[buf_size];
	double val;

	EXPECT_TRUE(MuggleStrTod("1.0", &val));
	EXPECT_DOUBLE_EQ(val, 1.0);
	EXPECT_TRUE(MuggleStrTod("20.0", &val));
	EXPECT_DOUBLE_EQ(val, 20.0);
	EXPECT_TRUE(MuggleStrTod("-15.0", &val));
	EXPECT_DOUBLE_EQ(val, -15.0);
	// snprintf(buf, buf_size, "%.10e", DBL_MAX);
	// EXPECT_TRUE(MuggleStrTod(buf, &val));
	// EXPECT_DOUBLE_EQ(val, DBL_MAX);
	snprintf(buf, buf_size, "%.10e", DBL_MIN);
	EXPECT_TRUE(MuggleStrTod(buf, &val));
	EXPECT_NEAR(val, FLT_MIN, DBL_EPSILON);

	EXPECT_FALSE(MuggleStrTod(NULL, &val));
	EXPECT_FALSE(MuggleStrTod("", NULL));
	EXPECT_FALSE(MuggleStrTod("", &val));
	EXPECT_FALSE(MuggleStrTod("1f", &val));
	EXPECT_FALSE(MuggleStrTod("1.1.1", &val));
	snprintf(buf, buf_size, "1%.10e", DBL_MAX);
	EXPECT_FALSE(MuggleStrTod(buf, &val));
}