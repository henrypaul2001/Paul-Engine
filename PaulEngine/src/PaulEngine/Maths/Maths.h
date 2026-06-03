#pragma once

#include <glm/glm.hpp>

namespace PaulEngine::Maths
{
	bool DecomposeTransform(const glm::mat4& transform, glm::vec3& out_translation, glm::vec3& out_rotation, glm::vec3& out_scale);
	glm::vec3 GetTranslation(const glm::mat4& transform);
	glm::vec3 GetRotation(const glm::mat4& transform);
	glm::vec3 GetScale(const glm::mat4& transform);
}