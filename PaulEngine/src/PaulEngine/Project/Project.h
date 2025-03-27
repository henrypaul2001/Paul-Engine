#pragma once
#include "PaulEngine/Asset/RuntimeAssetManager.h"
#include "PaulEngine/Asset/EditorAssetManager.h"

namespace PaulEngine
{
	struct ProjectSpecification
	{
		std::string Name = "Untitled Project";
		
		AssetHandle StartScene = 0;
		std::filesystem::path AssetDirectory;
		std::filesystem::path AssetRegistryPath;
		std::filesystem::path ProjectDirectory;
	};

	class Project
	{
	public:
		static const std::filesystem::path& GetProjectDirectory() {
			PE_CORE_ASSERT(s_ActiveProject, "No active project");
			return s_ActiveProject->m_ProjectDirectory;
		}
	
		static std::filesystem::path GetAssetRegistryPath() {
			PE_CORE_ASSERT(s_ActiveProject, "No active project");
			return GetAssetDirectory() / s_ActiveProject->m_Spec.AssetRegistryPath;
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

		Ref<AssetManagerBase> GetAssetManager() { return m_AssetManager; }
		Ref<RuntimeAssetManager> GetRuntimeAssetManager() { return std::static_pointer_cast<RuntimeAssetManager>(m_AssetManager); }
		Ref<EditorAssetManager> GetEditorAssetManager() { return std::static_pointer_cast<EditorAssetManager>(m_AssetManager); }

		static Ref<Project> New(const ProjectSpecification& spec);
		static Ref<Project> Load(const std::filesystem::path& path);
		static bool SaveActive(const std::filesystem::path& path);

	private:
		ProjectSpecification m_Spec;
		std::filesystem::path m_ProjectDirectory;
		Ref<AssetManagerBase> m_AssetManager;

		inline static Ref<Project> s_ActiveProject;
	};
}