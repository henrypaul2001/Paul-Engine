#pragma once
#include <PaulEngine/Core/Core.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace Utils
{
	inline void hash_combine(size_t& seed) {}

	// source: https://stackoverflow.com/a/38140932/26558868
	template<typename T, typename... Members>
	inline void hash_combine(size_t& seed, const T& v, Members... members)
	{
		std::hash<T> hasher;
		seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		hash_combine(seed, members...);
	}
}

#define MAKE_HASHABLE(type, ...) \
	template<>\
	struct std::hash<type> {\
		std::size_t operator()(const type& t) const noexcept\
		{\
			size_t result = 0;\
			Utils::hash_combine(result, __VA_ARGS__);\
			return result;\
		}\
	};\

MAKE_HASHABLE(glm::vec2, t.x, t.y)
MAKE_HASHABLE(glm::vec3, t.x, t.y, t.z)
MAKE_HASHABLE(glm::vec4, t.x, t.y, t.z, t.w)