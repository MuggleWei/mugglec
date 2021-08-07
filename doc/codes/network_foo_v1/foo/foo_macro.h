#ifndef NET_FOO_MACRO_H_
#define NET_FOO_MACRO_H_

#include "muggle/c/muggle_c.h"

#if WIN32 && defined(NET_FOO_USE_DLL)
	#ifdef NET_FOO_EXPORTS
		#define NET_FOO_EXPORT __declspec(dllexport)
	#else
		#define NET_FOO_EXPORT __declspec(dllimport)
	#endif
#else
	#define NET_FOO_EXPORT
#endif  // NET_FOO_EXPORT

#endif /* ifndef NET_FOO_MACRO_H_ */
