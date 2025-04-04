#include "pepch.h"
#include "ShaderImporter.h"
#include "PaulEngine/Project/Project.h"

namespace PaulEngine
{
	Ref<Shader> ShaderImporter::ImportShader(AssetHandle handle, const AssetMetadata& metadata)
	{
		PE_PROFILE_FUNCTION();
		Ref<Shader> shader = LoadShader(Project::GetAssetDirectory() / metadata.FilePath);
		shader->Handle = handle;
		return shader;
	}

	Ref<Shader> ShaderImporter::LoadShader(const std::filesystem::path& filepath)
	{
		PE_PROFILE_FUNCTION();
		return Shader::Create(filepath.string());
	}
}