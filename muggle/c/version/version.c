#include "version.h"
#include <stdio.h>

// extra level of indirection will allow the preprocessor to
// expand the macros before they are converted to strings
#define MUGGLE_STR_HELPER(x) #x
#define MUGGLE_STR(x)        MUGGLE_STR_HELPER(x)

#ifdef MUGGLE_C_VERSION_MAJOR
	#define MUGGLE_C_VERSION \
		MUGGLE_STR(MUGGLE_C_VERSION_MAJOR) "." \
		MUGGLE_STR(MUGGLE_C_VERSION_MINOR) "." \
		MUGGLE_STR(MUGGLE_C_VERSION_BUILD)
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
