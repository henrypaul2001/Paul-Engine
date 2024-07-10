#pragma once
#include <glm/ext/vector_float3.hpp>
#include <stack>
namespace Engine {
	class NavigationPath
	{
	public:
		NavigationPath() {}
		~NavigationPath() {}

		void AddWaypoint(const glm::vec3& position) { waypointStack.push(position); }
		void PopWaypointStack() { waypointStack.pop(); }

		const glm::vec3& GetNextWaypoint() const { return waypointStack.top(); }

		const std::stack<glm::vec3>& GetWaypointStack() const { return waypointStack; }
	private:
		std::stack<glm::vec3> waypointStack;
	};
}