#ifndef __RUN_TEST_EMPTY_H__
#define __RUN_TEST_EMPTY_H__

#if defined(RunTestEmpty_USE_DLL)
	#if RunTestEmpty_EXPORTS
		#define RunTestEmpty_EXPORT __declspec(dllexport)
	#else
		#define RunTestEmpty_EXPORT __declspec(dllimport)
	#endif
#else
	#define RunTestEmpty_EXPORT
#endif


RunTestEmpty_EXPORT void RunTestEmptyFunc();

#endif