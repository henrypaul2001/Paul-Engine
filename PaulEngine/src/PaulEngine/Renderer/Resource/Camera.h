#pragma once
#include <glm/glm.hpp>
#include <glm/ext.hpp>

namespace PaulEngine
{
	class Camera {
	public:
		Camera() {}
		virtual ~Camera() {}

		virtual const glm::mat4& GetProjection() const = 0;
		virtual float GetGamma() const = 0;
		virtual float GetExposure() const = 0;

	protected:
		//glm::mat4 m_Projection;
	};
}