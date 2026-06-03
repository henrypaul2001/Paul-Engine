#include "pepch.h"
#include "YamlConversions.h"

namespace PaulEngine
{
	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v) {
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}
	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v) {
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}
	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v) {
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}
	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::ivec2& v) {
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}
	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::ivec3& v) {
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}
	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::ivec4& v) {
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}
	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::uvec2& v) {
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}
	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::uvec3& v) {
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}
	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::uvec4& v) {
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}
	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::mat3& m) {
		out << YAML::Flow;
		out << YAML::BeginSeq << m[0][0] << m[0][1] << m[0][2] << YAML::EndSeq;
		out << YAML::BeginSeq << m[1][0] << m[1][1] << m[1][2] << YAML::EndSeq;
		out << YAML::BeginSeq << m[2][0] << m[2][1] << m[2][2] << YAML::EndSeq;
		return out;
	}
	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::mat4& m) {
		out << YAML::Flow;
		out << YAML::BeginSeq << m[0][0] << m[0][1] << m[0][2] << m[0][3] << YAML::EndSeq;
		out << YAML::BeginSeq << m[1][0] << m[1][1] << m[1][2] << m[1][3] << YAML::EndSeq;
		out << YAML::BeginSeq << m[2][0] << m[2][1] << m[2][2] << m[2][3] << YAML::EndSeq;
		out << YAML::BeginSeq << m[3][0] << m[3][1] << m[3][2] << m[3][3] << YAML::EndSeq;
		return out;
	}
}