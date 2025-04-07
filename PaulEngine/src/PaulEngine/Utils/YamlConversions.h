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
	struct convert<glm::ivec2>
	{
		static Node encode(const glm::ivec2& rhs) {
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			return node;
		}

		static bool decode(const Node& node, glm::ivec2& rhs) {
			if (!node.IsSequence() || node.size() != 2) {
				return false;
			}

			rhs.x = node[0].as<int>();
			rhs.y = node[1].as<int>();
			return true;
		}
	};

	template<>
	struct convert<glm::ivec3>
	{
		static Node encode(const glm::ivec3& rhs) {
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, glm::ivec3& rhs) {
			if (!node.IsSequence() || node.size() != 3) {
				return false;
			}

			rhs.x = node[0].as<int>();
			rhs.y = node[1].as<int>();
			rhs.z = node[2].as<int>();
			return true;
		}
	};

	template<>
	struct convert<glm::ivec4>
	{
		static Node encode(const glm::ivec4& rhs) {
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static bool decode(const Node& node, glm::ivec4& rhs) {
			if (!node.IsSequence() || node.size() != 4) {
				return false;
			}

			rhs.x = node[0].as<int>();
			rhs.y = node[1].as<int>();
			rhs.z = node[2].as<int>();
			rhs.w = node[3].as<int>();
			return true;
		}
	};

	template<>
	struct convert<glm::mat3>
	{
		static Node encode(const glm::mat3& rhs) {
			Node node;

			node.push_back(rhs[0][0]);
			node.push_back(rhs[0][1]);
			node.push_back(rhs[0][2]);

			node.push_back(rhs[1][0]);
			node.push_back(rhs[1][1]);
			node.push_back(rhs[1][2]);

			node.push_back(rhs[2][0]);
			node.push_back(rhs[2][1]);
			node.push_back(rhs[2][2]);

			return node;
		}

		static bool decode(const Node& node, glm::mat3& rhs) {
			if (!node.IsSequence() || node.size() != 9) {
				return false;
			}

			rhs[0][0] = node[0].as<float>();
			rhs[0][1] = node[1].as<float>();
			rhs[0][2] = node[2].as<float>();

			rhs[1][0] = node[3].as<float>();
			rhs[1][1] = node[4].as<float>();
			rhs[1][2] = node[5].as<float>();

			rhs[2][0] = node[6].as<float>();
			rhs[2][1] = node[7].as<float>();
			rhs[2][2] = node[8].as<float>();

			return true;
		}
	};

	template<>
	struct convert<glm::mat4>
	{
		static Node encode(const glm::mat4& rhs) {
			Node node;

			node.push_back(rhs[0][0]);
			node.push_back(rhs[0][1]);
			node.push_back(rhs[0][2]);
			node.push_back(rhs[0][3]);

			node.push_back(rhs[1][0]);
			node.push_back(rhs[1][1]);
			node.push_back(rhs[1][2]);
			node.push_back(rhs[1][3]);

			node.push_back(rhs[2][0]);
			node.push_back(rhs[2][1]);
			node.push_back(rhs[2][2]);
			node.push_back(rhs[2][3]);

			node.push_back(rhs[3][0]);
			node.push_back(rhs[3][1]);
			node.push_back(rhs[3][2]);
			node.push_back(rhs[3][3]);

			return node;
		}

		static bool decode(const Node& node, glm::mat4& rhs) {
			if (!node.IsSequence() || node.size() != 16) {
				return false;
			}

			rhs[0][0] = node[0].as<float>();
			rhs[0][1] = node[1].as<float>();
			rhs[0][2] = node[2].as<float>();
			rhs[0][3] = node[3].as<float>();

			rhs[1][0] = node[4].as<float>();
			rhs[1][1] = node[5].as<float>();
			rhs[1][2] = node[6].as<float>();
			rhs[1][3] = node[7].as<float>();

			rhs[2][0] = node[8].as<float>();
			rhs[2][1] = node[9].as<float>();
			rhs[2][2] = node[10].as<float>();
			rhs[2][3] = node[11].as<float>();

			rhs[3][0] = node[12].as<float>();
			rhs[3][1] = node[13].as<float>();
			rhs[3][2] = node[14].as<float>();
			rhs[3][3] = node[15].as<float>();

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