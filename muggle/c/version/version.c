#include "version.h"
#include <stdio.h>

#ifdef MUGGLE_CC_VERSION_MAJOR

#else
#define MUGGLE_CC_VERSION \
	"unknown " __DATE__ " " __TIME__
#endif

void mugglecc_version(char *buf, unsigned int bufsize)
{
#if defined(MUGGLE_CC_VERSION_MAJOR) && defined(MUGGLE_CC_VERSION_MINOR) && defined(MUGGLE_CC_VERSION_BUILD)
	snprintf(buf, bufsize, "%d.%d.%d",
		MUGGLE_CC_VERSION_MAJOR, MUGGLE_CC_VERSION_MINOR, MUGGLE_CC_VERSION_BUILD);
#else
	snprintf(buf, bufsize, "?.?.?");
#endif
}

void mugglecc_compile_time(char *buf, unsigned int bufsize)
{
	snprintf(buf, bufsize, __DATE__ " " __TIME__);
}
