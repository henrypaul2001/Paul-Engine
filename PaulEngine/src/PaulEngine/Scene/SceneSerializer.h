#pragma once
#include "Scene.h"

namespace PaulEngine
{
	class SceneSerializer
	{
	public:
		SceneSerializer(const Ref<Scene>& scene);

		void SerializeYAML(const std::filesystem::path& filepath, bool ignorePrefabChildren);
		void SerializeBinary(const std::filesystem::path& filepath);

		bool DeserializeYAML(const std::filesystem::path& filepath);
		bool DeserializeBinary(const std::filesystem::path& filepath);
	private:
		Ref<Scene> m_Scene;
	};
}