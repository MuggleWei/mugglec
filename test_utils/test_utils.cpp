#include "test_utils.h"
#include <string.h>
#include <stdexcept>

TestUtils::TestUtils()
{}
TestUtils::~TestUtils()
{}

int* TestUtils::allocateInteger()
{
	return (int*)malloc(sizeof(int));
}
void TestUtils::freeInteger(int *p)
{
	free(p);
}

char* TestUtils::allocateString()
{
	return (char*)malloc(sizeof(char) * 64);
}
void TestUtils::freeString(char *p)
{
	free(p);
}

int test_utils_cmp_int(const void *p1, const void *p2)
{
	if (p1 == p2)
	{
		return 0;
	}

	if (p1 == NULL && p2 != NULL)
	{
		return -1;
	}

	if (p1 != NULL && p2 == NULL)
	{
		return 1;
	}

	return *(int*)p1 - *(int*)p2;
}

int test_utils_cmp_str(const void *p1, const void *p2)
{
	if (p1 == p2)
	{
		return 0;
	}

	if (p1 == NULL && p2 != NULL)
	{
		return -1;
	}

	if (p1 != NULL && p2 == NULL)
	{
		return 1;
	}

	return strncmp((char*)p1, (char*)p2, TEST_UTILS_STR_SIZE);
}

void test_utils_free_int(void *pool, void *data)
{
	TestUtils *test_utils = (TestUtils*)pool;
	test_utils->freeInteger((int*)data);
}

void test_utils_free_str(void *pool, void *data)
{
	TestUtils *test_utils = (TestUtils*)pool;
	test_utils->freeString((char*)data);
}

