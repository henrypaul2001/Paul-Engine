#include "pepch.h"
#include "OrthographicCamera.h"

#include <glm/gtc/matrix_transform.hpp>
namespace PaulEngine {
	OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top) : m_ProjectionMatrix(glm::ortho(left, right, bottom, top, -1.0f, 1.0f)), m_Position(glm::vec3(0.0f)), m_Rotation(0.0f), 
		m_ViewMatrix(glm::mat4(1.0f)), m_ViewProjectionMatrix(m_ProjectionMatrix * m_ViewMatrix)
	{
	}

	void OrthographicCamera::RecalculateViewMatrix()
	{
		const glm::mat4 identity = glm::mat4(1.0f);
		glm::mat4 transform = glm::translate(identity, m_Position) *
			glm::rotate(identity, glm::radians(m_Rotation), glm::vec3(0.0f, 0.0f, 1.0f));

		m_ViewMatrix = glm::inverse(transform);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}
}