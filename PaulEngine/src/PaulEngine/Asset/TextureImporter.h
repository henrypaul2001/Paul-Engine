#pragma once
#include "Asset.h"

#include "PaulEngine/Renderer/Asset/Texture.h"
#include "PaulEngine/Renderer/Asset/TextureAtlas2D.h"
#include "PaulEngine/Renderer/Asset/EnvironmentMap.h"

namespace PaulEngine
{
	class TextureImporter
	{
	public:
		static Ref<Texture2D> ImportTexture2D(AssetHandle handle, const AssetMetadata& metadata);
		static Ref<Texture2D> LoadTexture2D(const std::filesystem::path& filepath);
		static Ref<Texture2D> LoadTexture2D(const std::filesystem::path& filepath, TextureSpecification spec);
		static Ref<Texture2D> LoadTexture2D(Buffer dataBuffer, TextureSpecification spec);

		static Ref<TextureAtlas2D> ImportTextureAtlas2D(AssetHandle handle, const AssetMetadata& metadata);
		static Ref<TextureAtlas2D> LoadTextureAtlas2D(const std::filesystem::path& filepath);
		static void SaveTextureAtlas2D(const TextureAtlas2D textureAtlas, const std::filesystem::path& filepath);

		static Ref<EnvironmentMap> ImportEnvironmentMap(AssetHandle handle, const AssetMetadata& metadata);
		static Ref<EnvironmentMap> LoadEnvironmentMap(const std::filesystem::path& filepath, bool persistentAsset = false);

		struct ImageFileReadResult
		{
			int Width;
			int Height;
			int Channels;

			bool operator ==(const ImageFileReadResult& other) {
				return (
					Width == other.Width &&
					Height == other.Height &&
					Channels == other.Channels
				);
			}

			bool operator !=(const ImageFileReadResult& other) {
				return (
					Width != other.Width ||
					Height != other.Height ||
					Channels != other.Channels
				);
			}
		};
		static Buffer ReadImageFile(const std::filesystem::path& filepath, ImageFileReadResult& out_result, bool flipVertical = true);

		static Ref<Texture2DArray> ImportTexture2DArray(AssetHandle handle, const AssetMetadata& metadata);
		static Ref<Texture2DArray> ReadBTAFile(const std::filesystem::path& filepath);
		static bool SaveBTAFile(const std::filesystem::path& filepath, std::vector<Buffer> layerBuffers, const TextureSpecification spec);
	};
}