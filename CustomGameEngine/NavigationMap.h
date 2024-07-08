#pragma once
#include <glm/ext/vector_float3.hpp>
#include "NavigationPath.h"
namespace Engine {
	class NavigationMap
	{
	public:
		NavigationMap() {}
		~NavigationMap() {}

		virtual bool FindPath(const glm::vec3& start, const glm::vec3& end, NavigationPath& out_path) = 0;
	};
}