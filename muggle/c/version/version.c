#include "version.h"
#include <stdio.h>

#ifdef MUGGLE_C_VERSION_MAJOR
	#define MUGGLE_C_VERSION \
		MUGGLE_MACRO_STR(MUGGLE_C_VERSION_MAJOR) "." \
		MUGGLE_MACRO_STR(MUGGLE_C_VERSION_MINOR) "." \
		MUGGLE_MACRO_STR(MUGGLE_C_VERSION_BUILD)
#else
	#define MUGGLE_C_VERSION "?.?.?"
#endif
#define MUGGLE_C_COMPILRE_TIME __DATE__ " " __TIME__

const char* mugglec_version()
{
	return MUGGLE_C_VERSION;
}

const char* mugglec_compile_time()
{
	return MUGGLE_C_COMPILRE_TIME;
}
