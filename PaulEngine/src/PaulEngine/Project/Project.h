#pragma once
namespace PaulEngine
{
	struct ProjectSpecification
	{
		std::string Name = "Untitled Project";
		
		std::filesystem::path StartScenePath;
		std::filesystem::path AssetDirectory;
	};

	class Project
	{
	public:
		static const std::filesystem::path& GetProjectDirectory() {
			PE_CORE_ASSERT(s_ActiveProject, "No active project");
			return s_ActiveProject->m_ProjectDirectory;
		}
		static std::filesystem::path GetAssetDirectory() {
			PE_CORE_ASSERT(s_ActiveProject, "No active project");
			return GetProjectDirectory() / s_ActiveProject->m_Spec.AssetDirectory;
		}

		static std::filesystem::path GetAssetFileSystemPath(const std::filesystem::path& path) {
			PE_CORE_ASSERT(s_ActiveProject, "No active project");
			return GetAssetDirectory() / path;
		}

		ProjectSpecification& GetSpecification() { return m_Spec; }
		static Ref<Project> GetActive() { return s_ActiveProject; }

		static Ref<Project> New();
		static Ref<Project> Load(const std::filesystem::path& path);
		static bool SaveActive(const std::filesystem::path& path);

	private:
		ProjectSpecification m_Spec;
		std::filesystem::path m_ProjectDirectory;

		inline static Ref<Project> s_ActiveProject;
	};
}