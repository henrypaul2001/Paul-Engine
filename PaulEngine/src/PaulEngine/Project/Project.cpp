#include "pepch.h"
#include "Project.h"
#include "ProjectSerializer.h"
#include "PaulEngine/Renderer/Renderer2D.h"
#include "PaulEngine/Renderer/Font.h"
#include "PaulEngine/Asset/AssetManager.h"

namespace PaulEngine
{
	Ref<Project> Project::New(const ProjectSpecification& spec)
	{
		s_ActiveProject = CreateRef<Project>();
		s_ActiveProject->m_Spec = spec;
		s_ActiveProject->m_ProjectDirectory = spec.ProjectDirectory.parent_path();
		Ref<EditorAssetManager> editorAssetManager = CreateRef<EditorAssetManager>();
		s_ActiveProject->m_AssetManager = editorAssetManager;

		// Create assets directory
		std::error_code error;
		std::filesystem::create_directories(GetAssetDirectory(), error);

		ImportEngineAssets();

		return s_ActiveProject;
	}

	Ref<Project> Project::Load(const std::filesystem::path& path)
	{
		Ref<Project> project = CreateRef<Project>();

		ProjectSerializer serializer = ProjectSerializer(project);

		if (serializer.Deserialize(path)) {
			project->m_ProjectDirectory = path.parent_path();
			s_ActiveProject = project;
			Ref<EditorAssetManager> editorAssetManager = CreateRef<EditorAssetManager>();
			s_ActiveProject->m_AssetManager = editorAssetManager;
			editorAssetManager->DeserializeAssetRegistry();

			ImportEngineAssets();

			return s_ActiveProject;
		}

		return nullptr;
	}

	Ref<Project> Project::LoadRuntime(const std::filesystem::path& path)
	{
		Ref<Project> project = CreateRef<Project>();

		ProjectSerializer serializer = ProjectSerializer(project);
		//serializer.DeserializeBinary(); TODO: runtime assets
		if (serializer.Deserialize(path)) {
			project->m_ProjectDirectory = path.parent_path();
			s_ActiveProject = project;

			Ref<RuntimeAssetManager> runtimeAssetManager = CreateRef<RuntimeAssetManager>();
			s_ActiveProject->m_AssetManager = runtimeAssetManager;
			runtimeAssetManager->DeserializeAssetRegistry();
			
			ImportEngineAssets();

			return s_ActiveProject;
		}

		return nullptr;
	}

	bool Project::SaveActive(const std::filesystem::path& path)
	{
		ProjectSerializer serializer = ProjectSerializer(s_ActiveProject);
		if (serializer.Serialize(path)) {
			s_ActiveProject->m_ProjectDirectory = path.parent_path();
			s_ActiveProject->GetEditorAssetManager()->SerializeAssetRegistry();
			return true;
		}

		return false;
	}

	void Project::ImportEngineAssets()
	{
		PE_CORE_ASSERT(s_ActiveProject, "Project cannot be null when importing engine assets");

		// Import engine assets
		std::filesystem::path fontRelativeToAssets = std::filesystem::path("assets/fonts/Open_Sans/static/OpenSans-Regular.ttf").lexically_relative(Project::GetAssetDirectory());
		Font::s_DefaultFont = Project::GetActive()->GetEditorAssetManager()->ImportAsset(fontRelativeToAssets, true);

		Renderer2D::ImportShaders();
	}
}