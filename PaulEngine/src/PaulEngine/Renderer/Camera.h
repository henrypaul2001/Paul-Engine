#pragma once
#include <glm/glm.hpp>
#include <glm/ext.hpp>

namespace PaulEngine
{
	class Camera {
	public:
		Camera(const glm::mat4& projection = glm::mat4(1.0f)) : m_Projection(projection) {}
		virtual ~Camera() {}

		const glm::mat4& GetProjection() const { return m_Projection; }

	protected:
		glm::mat4 m_Projection;
	};
}