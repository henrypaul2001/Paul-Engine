#include "pepch.h"
#include "EnvironmentMap.h"

#include "PaulEngine/Asset/AssetManager.h"
#include "PaulEngine/Asset/TextureImporter.h"

constexpr uint32_t CUBE_MAP_RESOLUTION = 512u;

namespace PaulEngine
{
	EnvironmentMap::EnvironmentMap(const std::filesystem::path& hdrPath)
	{
		PE_PROFILE_FUNCTION();
		PE_CORE_ASSERT(hdrPath.extension() == ".hdr", "Invalid file extension");

		TextureSpecification spec;
		spec.Format = ImageFormat::RGB32F;
		spec.Wrap_S = ImageWrap::CLAMP_TO_EDGE;
		spec.Wrap_T = ImageWrap::CLAMP_TO_EDGE;
		spec.MinFilter = ImageMinFilter::LINEAR;
		spec.MagFilter = ImageMagFilter::LINEAR;
		Ref<Texture2D> hdrTexture = TextureImporter::LoadTexture2D(hdrPath, spec);

		spec = TextureSpecification();
		spec.Format = ImageFormat::RGB32F;
		spec.Width = CUBE_MAP_RESOLUTION;
		spec.Height = CUBE_MAP_RESOLUTION;
		spec.Wrap_S = ImageWrap::CLAMP_TO_EDGE;
		spec.Wrap_T = ImageWrap::CLAMP_TO_EDGE;
		spec.Wrap_R = ImageWrap::CLAMP_TO_EDGE;
		spec.MinFilter = ImageMinFilter::LINEAR;
		spec.MagFilter = ImageMagFilter::LINEAR;

		// TODO: implement mipmap generation in OpenGL textures
		spec.GenerateMips = true;
		m_BaseCubemap = TextureCubemap::Create(spec);

		spec.MinFilter = ImageMinFilter::LINEAR_MIPMAP_LINEAR;
		m_PrefilteredCubemap = TextureCubemap::Create(spec);

		spec.Width = 32;
		spec.Height = 32;
		spec.GenerateMips = false;
		m_IrradianceCubemap = TextureCubemap::Create(spec);

		// Convert loaded equirectangular hdr texture to a cubemap texture
		ConvertEquirectangularToCubemap(hdrTexture, m_BaseCubemap);

		ConvoluteEnvironmentMap(m_BaseCubemap, m_IrradianceCubemap);

		PrefilterEnvironmentMap(m_BaseCubemap, m_PrefilteredCubemap);
	}

	void EnvironmentMap::ConvertEquirectangularToCubemap(Ref<Texture2D> equirectangular, Ref<TextureCubemap> targetCubemap)
	{
		PE_PROFILE_FUNCTION();

	}

	void EnvironmentMap::ConvoluteEnvironmentMap(Ref<TextureCubemap> environmentMap, Ref<TextureCubemap> targetCubemap)
	{
		PE_PROFILE_FUNCTION();

	}

	void EnvironmentMap::PrefilterEnvironmentMap(Ref<TextureCubemap> environmentMap, Ref<TextureCubemap> targetCubemap)
	{
		PE_PROFILE_FUNCTION();

	}
}