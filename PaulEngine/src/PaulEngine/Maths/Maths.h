#pragma once

#include <glm/glm.hpp>

namespace PaulEngine::Maths
{
	bool DecomposeTransform(const glm::mat4& transform, glm::vec3& out_translation, glm::vec3& out_rotation, glm::vec3& out_scale);
}