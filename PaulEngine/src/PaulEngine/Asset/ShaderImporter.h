#pragma once
#include "Asset.h"
#include "PaulEngine/Renderer/Asset/Shader.h"

namespace PaulEngine
{
	class ShaderImporter
	{
	public:
		static Ref<Shader> ImportShader(AssetHandle handle, const AssetMetadata& metadata);
		static Ref<Shader> LoadShader(const std::filesystem::path& filepath);
	};
}