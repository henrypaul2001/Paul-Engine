#include "pepch.h"
#include "TextureImporter.h"

#include <stb_image.h>
#include "PaulEngine/Project/Project.h"

namespace PaulEngine
{
	Ref<Texture2D> TextureImporter::ImportTexture2D(AssetHandle handle, const AssetMetadata& metadata)
	{
		PE_PROFILE_FUNCTION();
		Ref<Texture2D> texture = LoadTexture2D(Project::GetAssetDirectory() / metadata.FilePath);
		texture->Handle = handle;
		return texture;
	}

	Ref<Texture2D> TextureImporter::LoadTexture2D(const std::filesystem::path& filepath)
	{
		PE_PROFILE_FUNCTION();

		int width, height, channels;
		stbi_set_flip_vertically_on_load(true);

		unsigned char* imageData = nullptr;
		{
			PE_PROFILE_SCOPE("stbi_load image");
			std::string pathString = filepath.string();
			imageData = stbi_load(pathString.c_str(), &width, &height, &channels, 0);
		}

		if (imageData == nullptr)
		{
			PE_CORE_ERROR("Failed to load image at path: '{0}'", filepath.string());
			return nullptr;
		}

		Buffer data = Buffer(imageData, sizeof(unsigned char) * (width * height * channels));

		TextureSpecification spec;
		spec.Width = width;
		spec.Height = height;

		switch (channels)
		{
		case 4:
			spec.Format = ImageFormat::RGBA8;
			break;
		case 3:
			spec.Format = ImageFormat::RGB8;
			break;
		case 2:
			spec.Format = ImageFormat::RG8;
			break;
		case 1:
			spec.Format = ImageFormat::R8;
			break;
		}

		Ref<Texture2D> texture = Texture2D::Create(spec, data);
		data.Release();

		return texture;
	}
}