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

#ifdef PE_DEBUG
	#define PE_ENABLE_ASSERTS
#endif

#ifdef PE_ENABLE_ASSERTS
	#define PE_ASSERT(x, ...) { if(!(x)) { PE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); }}
	#define PE_CORE_ASSERT(x, ...) { if(!(x)) { PE_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); }}
#else
	#define PE_ASSERT(x, ...)
	#define PE_CORE_ASSERT(x, ...)
#endif

#define BIT(x) (1 << x)

#define PE_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)
#define PE_BIND_EVENT_FN_PTR(fn, ptr) std::bind(&fn, ptr, std::placeholders::_1)