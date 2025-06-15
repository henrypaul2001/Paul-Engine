#pragma once
#include "PaulEngine/Asset/Asset.h"
#include "PaulEngine/Renderer/Asset/Texture.h"
#include "PaulEngine/Renderer/Resource/Framebuffer.h"

#include <filesystem>

namespace PaulEngine
{
	class EnvironmentMap : public Asset
	{
	public:
		// Generates an environment map: basecubemap, irradiance cubemap and prefiltered cubemap from a .hdr equirectangular source file
		// No caching yet
		EnvironmentMap(const std::filesystem::path& hdrPath, bool persistentAsset = false);

		virtual AssetType GetType() const override { return AssetType::EnvironmentMap; }

		const Ref<TextureCubemap> GetUnfiltered() const { return m_BaseCubemap; }
		const Ref<TextureCubemap> GetIrradianceMap() const { return m_IrradianceCubemap; }
		const Ref<TextureCubemap> GetPrefilteredMap() const { return m_PrefilteredCubemap; }

		static void ConvertEquirectangularToCubemap(Ref<Texture2D> equirectangular, AssetHandle targetCubemapHandle);
		static void ConvoluteEnvironmentMap(Ref<TextureCubemap> environmentMap, AssetHandle targetCubemapHandle);
		static void PrefilterEnvironmentMap(Ref<TextureCubemap> environmentMap, AssetHandle targetCubemapHandle);
	private:
		static void InitCubeCaptureFBO();

		Ref<TextureCubemap> m_BaseCubemap;
		Ref<TextureCubemap> m_IrradianceCubemap;
		Ref<TextureCubemap> m_PrefilteredCubemap;

		static Ref<Framebuffer> s_CubeCaptureFBO;
		static AssetHandle s_ConvertToCubemapShaderHandle;
		static AssetHandle s_ConvertToCubemapMaterialHandle;
		static AssetHandle s_ConvolutionShaderHandle;
		static AssetHandle s_ConvolutionMaterialHandle;
		static AssetHandle s_PrefilterShaderHandle;
		static AssetHandle s_PrefilterMaterialHandle;
	};
}

/*

class EnvironmentMap : public Asset
	AssetType::EnvironmentMap

AssetMetadata filepath points to an equirectangular .hdr file

- When importing this asset, the base hdr file will be converted to a cubemap stored inside the environment map class
- Then the generated cubemap will be processed such as convolution which will also be stored in the environment map class
  as a seperate cubemap

- The processed cubemaps will be cached in the engine assets/cache/ibl/(HDRName)/ folder similar to how shaders are cached
- In the future, there will be a custom cubemap asset file type. So, for now, these cached cubemaps will be stored as individual faces
*/