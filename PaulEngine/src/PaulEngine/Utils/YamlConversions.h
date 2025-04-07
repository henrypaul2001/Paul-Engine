#pragma once
#include "PaulEngine/Core/UUID.h"
#include <yaml-cpp/yaml.h>
#include <glm/glm.hpp>

// Type encode / decode
namespace YAML
{
	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs) {
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs) {
			if (!node.IsSequence() || node.size() != 2) {
				return false;
			}

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs) {
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs) {
			if (!node.IsSequence() || node.size() != 3) {
				return false;
			}

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& rhs) {
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs) {
			if (!node.IsSequence() || node.size() != 4) {
				return false;
			}

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};

	template<>
	struct convert<PaulEngine::UUID>
	{
		static Node encode(const PaulEngine::UUID& uuid) {
			Node node;
			node.push_back((uint64_t)uuid);
			return node;
		}

		static bool decode(const Node& node, PaulEngine::UUID& uuid) {
			uuid = node.as<uint64_t>();
			return true;
		}
	};
}

namespace PaulEngine
{
	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v);
	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v);
	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v);
	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::ivec2& v);
	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::ivec3& v);
	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::ivec4& v);
	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::mat3& m);
	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::mat4& m);
}