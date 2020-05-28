#include "version.h"
#include <stdio.h>

#ifdef MUGGLE_CC_VERSION_MAJOR

#else
#define MUGGLE_CC_VERSION \
	"unknown " __DATE__ " " __TIME__
#endif

void mugglecc_version_info(char *buf, unsigned int bufsize)
{
#if defined(MUGGLE_CC_VERSION_MAJOR) && defined(MUGGLE_CC_VERSION_MINOR) && defined(MUGGLE_CC_VERSION_BUILD)
	snprintf(buf, bufsize, "%d.%d.%d - " __DATE__ " " __TIME__,
		MUGGLE_CC_VERSION_MAJOR, MUGGLE_CC_VERSION_MINOR, MUGGLE_CC_VERSION_BUILD);
#else
	snprintf(buf, bufsize, "?.?.? - " __DATE__ " " __TIME__);
#endif
}
