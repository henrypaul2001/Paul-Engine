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
			uint32_t Width;
			uint32_t Height;
			uint8_t Channels;
			uint8_t BytesPerPixel;

			bool operator ==(const ImageFileReadResult& other) {
				return (
					Width == other.Width &&
					Height == other.Height &&
					Channels == other.Channels &&
					BytesPerPixel == other.BytesPerPixel
				);
			}

			bool operator !=(const ImageFileReadResult& other) {
				return (
					Width != other.Width ||
					Height != other.Height ||
					Channels != other.Channels ||
					BytesPerPixel != other.BytesPerPixel
				);
			}
		};
		static Buffer ReadImageFile(const std::filesystem::path& filepath, ImageFileReadResult& out_result, bool flipVertical = true);
		static bool SaveImageFile(std::filesystem::path filepath, const Buffer pixelData, const TextureSpecification spec, bool flipVertical = true, const uint8_t jpgQualityLevel = 50);
		
		static Ref<Texture2DArray> ImportTexture2DArray(AssetHandle handle, const AssetMetadata& metadata);
		static Ref<Texture2DArray> ReadBTAFile(const std::filesystem::path& filepath);
		static bool SaveBTAFile(const std::filesystem::path& filepath, std::vector<Buffer> layerBuffers, const TextureSpecification spec);

		static Ref<TextureCubemap> ImportTextureCubemap(AssetHandle handle, const AssetMetadata& metadata);
		static Ref<TextureCubemap> LoadTextureCubemap(const std::filesystem::path& filepath);
		/*
		0: positiveX
		1: negativeX
		2: positiveY
		3: negativeY
		4: positiveZ
		5: negativeZ
		*/
		static bool SaveCubemapFile(const std::filesystem::path& filepath, const Buffer uncompressedFaces[6], const TextureSpecification spec);
	
		static bool SaveImagePNG(const std::filesystem::path& filepath, const Buffer pixelData, const TextureSpecification spec);
		static bool SaveImageBMP(const std::filesystem::path& filepath, const Buffer pixelData, const TextureSpecification spec);
		static bool SaveImageTGA(const std::filesystem::path& filepath, const Buffer pixelData, const TextureSpecification spec);
		// qualityLevel { 1, 100 }
		static bool SaveImageJPG(const std::filesystem::path& filepath, const Buffer pixelData, const TextureSpecification spec, const uint8_t qualityLevel = 50);
		static bool SaveImageHDR(const std::filesystem::path& filepath, const Buffer pixelData, const TextureSpecification spec);

	private:
		// TODO: Implement some kind of templated struct type with encode and decode similar to YAML template overrides for custom serialization
		static void WriteTextureSpecBinary(std::ofstream& fout, const TextureSpecification& spec);
		static TextureSpecification ReadTextureSpecBinary(std::ifstream& fin);
	};
}