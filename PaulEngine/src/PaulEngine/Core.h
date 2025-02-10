#pragma once
#ifdef PE_PLATFORM_WINDOWS
	#ifdef PE_BUILD_DLL
		#define PAUL_API __declspec(dllexport)
	#else
		#define PAUL_API __declspec(dllimport)
	#endif
#else
	#error PaulEngine only supports Windows
#endif

#define BIT(x) (1 << x)