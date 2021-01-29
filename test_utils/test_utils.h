#ifndef MUGGLE_C_TEST_UTILS_H_
#define MUGGLE_C_TEST_UTILS_H_

#if MUGGLE_PLATFORM_WINDOWS && defined(test_utils_USE_DLL)
	#ifdef test_utils_EXPORTS
		#define TEST_UTILS_EXPORT __declspec(dllexport)
	#else
		#define TEST_UTILS_EXPORT __declspec(dllimport)
	#endif
#else
	#define TEST_UTILS_EXPORT
#endif

#define TEST_UTILS_STR_SIZE 64

class TestUtils
{
public:
	TEST_UTILS_EXPORT
	TestUtils();
	TEST_UTILS_EXPORT
	virtual ~TestUtils();

	TEST_UTILS_EXPORT
	int* allocateInteger();
	TEST_UTILS_EXPORT
	void freeInteger(int *p);

	TEST_UTILS_EXPORT
	char* allocateString();
	TEST_UTILS_EXPORT
	void freeString(char *p);
};

TEST_UTILS_EXPORT
int test_utils_cmp_int(const void *p1, const void *p2);

TEST_UTILS_EXPORT
int test_utils_cmp_str(const void *p1, const void *p2);

TEST_UTILS_EXPORT
void test_utils_free_int(void *pool, void *data);

TEST_UTILS_EXPORT
void test_utils_free_str(void *pool, void *data);

#endif

