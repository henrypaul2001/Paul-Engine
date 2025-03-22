#pragma once
#include "Asset.h"

#include "PaulEngine/Renderer/Texture.h"

namespace PaulEngine
{
	class TextureImporter
	{
	public:
		static Ref<Texture2D> ImportTexture2D(AssetHandle handle, const AssetMetadata& metadata);
	};
}