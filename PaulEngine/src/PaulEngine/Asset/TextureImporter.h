#pragma once
#include "Asset.h"

#include "PaulEngine/Renderer/Texture.h"
#include "PaulEngine/Renderer/TextureAtlas2D.h"

namespace PaulEngine
{
	class TextureImporter
	{
	public:
		static Ref<Texture2D> ImportTexture2D(AssetHandle handle, const AssetMetadata& metadata);
		static Ref<Texture2D> LoadTexture2D(const std::filesystem::path& filepath);

		static Ref<TextureAtlas2D> ImportTextureAtlas2D(AssetHandle handle, const AssetMetadata& metadata);
		static Ref<TextureAtlas2D> LoadTextureAtlas2D(const std::filesystem::path& filepath);
		static void SaveTextureAtlas2D(const TextureAtlas2D textureAtlas, const std::filesystem::path& filepath);

		struct ImageFileReadResult
		{
			int Width;
			int Height;
			int Channels;
		};
		static Buffer ReadImageFile(const std::filesystem::path& filepath, ImageFileReadResult& out_result, bool flipVertical = true);

		static Ref<Texture2DArray> ReadBTAFile(const std::filesystem::path& filepath);
		static bool SaveBTAFile(const std::filesystem::path& filepath, std::vector<Buffer> layerBuffers, const TextureSpecification spec);
	};
}