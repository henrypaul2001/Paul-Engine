#pragma once
#include <glm/ext/vector_float3.hpp>
namespace Engine {
	class ReflectionProbe
	{
	public:
		ReflectionProbe(const unsigned int id, const glm::vec3& position);
		~ReflectionProbe();

	private:
		unsigned int fileID;
		glm::vec3 worldPosition;
	};
}