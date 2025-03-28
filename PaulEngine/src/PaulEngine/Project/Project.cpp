#include "pepch.h"
#include "Project.h"
#include "ProjectSerializer.h"

namespace PaulEngine
{
	Ref<Project> Project::New(const ProjectSpecification& spec)
	{
		s_ActiveProject = CreateRef<Project>();
		s_ActiveProject->m_Spec = spec;
		s_ActiveProject->m_ProjectDirectory = spec.ProjectDirectory.parent_path();
		Ref<EditorAssetManager> editorAssetManager = CreateRef<EditorAssetManager>();
		s_ActiveProject->m_AssetManager = editorAssetManager;
		return s_ActiveProject;
	}

	Ref<Project> Project::Load(const std::filesystem::path& path, const bool runtime)
	{
		Ref<Project> project = CreateRef<Project>();

		ProjectSerializer serializer = ProjectSerializer(project);

		if (runtime)
		{
			//serializer.DeserializeBinary(); TODO: runtime assets
			if (serializer.Deserialize(path)) {
				project->m_ProjectDirectory = path.parent_path();
				s_ActiveProject = project;

				Ref<RuntimeAssetManager> runtimeAssetManager = CreateRef<RuntimeAssetManager>();
				s_ActiveProject->m_AssetManager = runtimeAssetManager;

				runtimeAssetManager->DeserializeAssetRegistry();
				return s_ActiveProject;
			}
		}
		else {

			if (serializer.Deserialize(path)) {
				project->m_ProjectDirectory = path.parent_path();
				s_ActiveProject = project;
				Ref<EditorAssetManager> editorAssetManager = CreateRef<EditorAssetManager>();
				s_ActiveProject->m_AssetManager = editorAssetManager;
				editorAssetManager->DeserializeAssetRegistry();
				return s_ActiveProject;
			}
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
}