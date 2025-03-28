#pragma once
#include "Project.h"

namespace PaulEngine
{
	class ProjectSerializer
	{
	public:
		ProjectSerializer(Ref<Project> project);

		bool Serialize(const std::filesystem::path& filepath);
		bool Deserialize(const std::filesystem::path& filepath);

		// bool SerializeBinary();
		// bool DeserializeBinary();

	private:
		Ref<Project> m_Project;
	};
}