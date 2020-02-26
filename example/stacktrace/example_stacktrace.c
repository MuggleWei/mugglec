#include "muggle/c/muggle_c.h"

void dummy_function(int idx, muggle_stacktrace_t *st);

void example_get_stacktrace(int idx, muggle_stacktrace_t *st)
{
	if (idx <= 0)
	{
		MUGGLE_INFO("========= muggle_print_stacktrace =========");
		muggle_print_stacktrace();
		printf("\n");

		if (muggle_stacktrace_get(st, 0) == -1)
		{
			MUGGLE_ERROR("failed get stacktrace");
			exit(EXIT_FAILURE);
		}
	}
	else
	{
		dummy_function(idx-1, st);
	}
}

void dummy_function(int idx, muggle_stacktrace_t *st)
{
	example_get_stacktrace(idx, st);
}

int main()
{
	if (muggle_log_simple_init(MUGGLE_LOG_LEVEL_INFO, MUGGLE_LOG_LEVEL_INFO) != 0)
	{
		MUGGLE_ERROR("failed initalize log");
		exit(EXIT_FAILURE);
	}

	muggle_stacktrace_t st;
	example_get_stacktrace(8, &st);

	MUGGLE_INFO("========= muggle_stacktrace_get =========");
	for (unsigned int i = 0; i < st.cnt_frame; ++i)
	{
		MUGGLE_INFO("#%u %s", i, st.symbols[i]);
	}

	muggle_stacktrace_free(&st);

	return 0;
}
