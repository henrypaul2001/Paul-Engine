#pragma once
#include "PaulEngine/Asset/Asset.h"
#include "PaulEngine/Asset/BinarySerializer.h"
#include "PaulEngine/Renderer/Asset/Texture.h"
#include "PaulEngine/Renderer/Resource/Framebuffer.h"

#include <filesystem>

namespace PaulEngine
{
	class EnvironmentMap : public Asset
	{
	public:
		EnvironmentMap() : m_BaseCubemapHandle(0), m_IrradianceCubemapHandle(0), m_PrefilteredCubemapHandle(0) {}

		// Generates an environment map: basecubemap, irradiance cubemap and prefiltered cubemap from a .hdr equirectangular source file
		EnvironmentMap(const std::filesystem::path& hdrPath, bool persistentAsset = false);

		EnvironmentMap(const std::filesystem::path& basePath, const std::filesystem::path& irradiancePath, const std::filesystem::path& prefilteredPath, bool persistentAsset = false);

		EnvironmentMap(const AssetHandle baseCubemapHandle, const AssetHandle irradianceMapHandle, const AssetHandle prefilteredMapHandle);

		virtual AssetType GetType() const override { return AssetType::EnvironmentMap; }

		const AssetHandle GetUnfilteredHandle() const { return m_BaseCubemapHandle; }
		const AssetHandle GetIrradianceMapHandle() const { return m_IrradianceCubemapHandle; }
		const AssetHandle GetPrefilteredMapHandle() const { return m_PrefilteredCubemapHandle; }

		static AssetHandle GetBRDFLutHandle() { 
			if (!AssetManager::IsAssetHandleValid(s_BRDFLutTextureHandle)) { GenerateBRDFLut(); }
			return s_BRDFLutTextureHandle;
		}

		static void ConvertEquirectangularToCubemap(Ref<Texture2D> equirectangular, AssetHandle targetCubemapHandle);
		static void ConvoluteEnvironmentMap(Ref<TextureCubemap> environmentMap, AssetHandle targetCubemapHandle);
		static void PrefilterEnvironmentMap(Ref<TextureCubemap> environmentMap, AssetHandle targetCubemapHandle);
	private:
		friend class BinarySerializer;

		void CacheCubemaps(const std::filesystem::path& cubemapDirectory, const std::string& baseName);
		void CacheCubemap(const AssetHandle cubemapHandle, const std::filesystem::path& cubemapPath);

		static void InitEnvMapProcessing();
		static void GenerateBRDFLut();

		AssetHandle m_BaseCubemapHandle;
		AssetHandle m_IrradianceCubemapHandle;
		AssetHandle m_PrefilteredCubemapHandle;

		static Ref<Framebuffer> s_CubeCaptureFBO;
		static AssetHandle s_ConvertToCubemapShaderHandle;
		static AssetHandle s_ConvertToCubemapMaterialHandle;
		static AssetHandle s_ConvolutionShaderHandle;
		static AssetHandle s_ConvolutionMaterialHandle;
		static AssetHandle s_PrefilterShaderHandle;
		static AssetHandle s_PrefilterMaterialHandle;
		static AssetHandle s_BRDFLutShaderHandle;
		static AssetHandle s_BRDFLutMaterialHandle;
		static AssetHandle s_BRDFLutTextureHandle;
	};

	template <>
	size_t BinarySerializer::SerializeAssetBinary<EnvironmentMap>(EnvironmentMap& asset, std::ostream& stream);

	template <>
	bool BinarySerializer::DeserializeAssetBinaryData<EnvironmentMap>(EnvironmentMap& asset, std::istream& stream);
}