#pragma once
#include <glm/glm.hpp>
#include <glm/ext.hpp>

namespace PaulEngine
{
	class Camera {
	public:
		Camera() { SetPerspective(90.0f, 1.6f / 0.9f); }
		Camera(const glm::mat4& projection) : m_Projection(projection) {}

		const glm::mat4& GetProjection() const { return m_Projection; }

		void SetPerspective(const float vfov, const float aspect, const float nearClip = 0.01f, const float farClip = 1000.0f) {
			m_Projection = glm::perspective(vfov, aspect, nearClip, farClip);
		}
		void SetOrthographic(const float left, const float right, const float bottom, const float top, const float nearClip = 0.01f, const float farClip = 1000.0f) {
			m_Projection = glm::ortho(left, right, bottom, top, nearClip, farClip);
		}

		static Camera CreatePerspectiveCamera(const float vfov, const float aspect, const float nearClip = 0.01f, const float farClip = 1000.0f) { return Camera(glm::perspective(vfov, aspect, nearClip, farClip)); }
		static Camera CreateOrthographicCamera(const float left, const float right, const float bottom, const float top, const float nearClip = 0.01f, const float farClip = 1000.0f) { return Camera(glm::ortho(left, right, bottom, top, nearClip, farClip)); }

	private:
		glm::mat4 m_Projection;
	};
}