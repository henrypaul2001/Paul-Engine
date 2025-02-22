#pragma once
#include <memory>

#ifdef PE_PLATFORM_WINDOWS
#else
	#error PaulEngine only supports Windows
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

namespace PaulEngine {
	template <typename T>
	// (std::unique_ptr)
	using Scope = std::unique_ptr<T>;
	template <typename T, typename ...Args>
	constexpr Scope<T> CreateScope(Args&& ... args) {
		return std::make_unique<T>(std::forward<Args>(args)...);
	}
	
	// Room for expansion into custom pointer types
	/*
	template <typename T>
	class Scope {

	};
	*/

	template <typename T>
	// (std::shared_ptr)
	using Ref = std::shared_ptr<T>;
	template <typename T, typename ...Args>
	constexpr Ref<T> CreateRef(Args&& ... args) {
		return std::make_shared<T>(std::forward<Args>(args)...);
	}
}