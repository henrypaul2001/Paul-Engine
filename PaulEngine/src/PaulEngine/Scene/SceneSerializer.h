#pragma once
#include "Scene.h"

namespace PaulEngine
{
	class SceneSerializer
	{
	public:
		SceneSerializer(const Ref<Scene>& scene);

		void SerializeYAML(const std::string& filepath);
		void SerializeBinary(const std::string& filepath);

		bool DeserializeYAML(const std::string& filepath);
		bool DeserializeBinary(const std::string& filepath);
	private:
		Ref<Scene> m_Scene;
	};
}