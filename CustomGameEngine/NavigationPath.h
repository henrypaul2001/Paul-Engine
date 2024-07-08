#pragma once
#include <glm/ext/vector_float3.hpp>
#include <vector>
namespace Engine {
	class NavigationPath
	{
	public:
		NavigationPath() {}
		~NavigationPath() {}

		void AddWaypoint(const glm::vec3& position) { waypoints.push_back(position); }
	private:
		std::vector<glm::vec3> waypoints;
	};
}