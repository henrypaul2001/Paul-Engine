#pragma once
#include "PaulEngine/Asset/Asset.h"
#include "PaulEngine/Core/Core.h"
#include "PaulEngine/Core/Buffer.h"

#include <glm/glm.hpp>
#include <string>
namespace PaulEngine {
	enum class CubemapFace
	{
		POSITIVE_X = 0,
		NEGATIVE_X = 1,
		POSITIVE_Y = 2,
		NEGATIVE_Y = 3,
		POSITIVE_Z = 4,
		NEGATIVE_Z = 5
	};

	enum class ImageFormat
	{
		None = 0,
		Depth16,
		Depth24,
		Depth32,
		Depth24Stencil8,
		RED_INTEGER,
		R8,
		RG8,
		RGB8,
		RGBA8,
		R11FG11FB10F,
		RG16F,
		RGB16F,
		RGBA16F,
		RG32F,
		RGB32F,
		RGBA32F
	};

	static bool IsIntegerFormat(const ImageFormat format)
	{
		switch (format)
		{
			case ImageFormat::RED_INTEGER: return true;
		}

		return false;
	}

	static const std::unordered_map<ImageFormat, int> s_FormatChannels = {
		{ ImageFormat::None,			0 },
		{ ImageFormat::Depth16,			1 },
		{ ImageFormat::Depth24,			1 },
		{ ImageFormat::Depth32,			1 },
		{ ImageFormat::Depth24Stencil8, 2 },
		{ ImageFormat::RED_INTEGER,		1 },
		{ ImageFormat::R8,				1 },
		{ ImageFormat::RG8,				2 },
		{ ImageFormat::RGB8,			3 },
		{ ImageFormat::RGBA8,			4 },
		{ ImageFormat::R11FG11FB10F,	3 },
		{ ImageFormat::RG16F,			2 },
		{ ImageFormat::RGB16F,			3 },
		{ ImageFormat::RGBA16F,			4 },
		{ ImageFormat::RG32F,			2 },
		{ ImageFormat::RGB32F,			3 },
		{ ImageFormat::RGBA32F,			4 }
	};

	// Verifies channel count against requested image format.
	// numChannels value set to real channel count of image format
	static bool ValidateChannels(ImageFormat format, int& numChannels)
	{
		auto it = s_FormatChannels.find(format);
		PE_CORE_ASSERT(it != s_FormatChannels.end(), "Undefined channel translation");
		bool result = (it->second == numChannels);
		numChannels = it->second;
		return result;
	}

	enum class ImageMinFilter
	{
		None = 0,
		NEAREST,
		LINEAR,
		NEAREST_MIPMAP_NEAREST,
		LINEAR_MIPMAP_NEAREST,
		NEAREST_MIPMAP_LINEAR,
		LINEAR_MIPMAP_LINEAR
	};

	enum class ImageMagFilter
	{
		None = 0,
		NEAREST,
		LINEAR
	};

	enum class ImageWrap
	{
		None = 0,
		CLAMP_TO_EDGE,
		CLAMP_TO_BORDER,
		MIRRORED_REPEAT,
		REPEAT,
		MIRROR_CLAMP_TO_EDGE
	};

	struct TextureSpecification
	{
		uint32_t Width = 1;
		uint32_t Height = 1;
		ImageFormat Format = ImageFormat::RGBA8;
		ImageMinFilter MinFilter = ImageMinFilter::LINEAR_MIPMAP_LINEAR;
		ImageMagFilter MagFilter = ImageMagFilter::LINEAR;
		ImageWrap Wrap_S = ImageWrap::REPEAT;
		ImageWrap Wrap_T = ImageWrap::REPEAT;
		ImageWrap Wrap_R = ImageWrap::REPEAT;
		glm::vec4 Border = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		bool GenerateMips = true;
	};

	static int NumChannels(ImageFormat format)
	{
		auto it = s_FormatChannels.find(format);
		PE_CORE_ASSERT(it != s_FormatChannels.end(), "Undefined channel translation");
		return (it->second);
	}

	class Texture : public Asset
	{
	public:
		virtual ~Texture() = default;

		virtual const TextureSpecification& GetSpecification() const = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetRendererID() const = 0;

		virtual void SetData(Buffer data) = 0;
		virtual void Clear(int value) = 0;
		virtual void Clear(float value) = 0;

		virtual void GenerateMipmaps() = 0;
		virtual void Resize(uint32_t width, uint32_t height) = 0;

		virtual void Bind(const uint32_t slot = 0) const = 0;
		virtual uint64_t GetDeviceTextureHandle() const = 0;

		virtual bool operator==(const Texture& other) const = 0;
	};

	class Texture2D : public Texture
	{
	public:
		static Ref<Texture2D> Create(const TextureSpecification& specification, Buffer data = Buffer());

		static AssetType GetStaticType() { return AssetType::Texture2D; }
		virtual AssetType GetType() const override { return GetStaticType(); }

		static void CreateTextures(bool multisampled, uint32_t* out_ID, uint32_t count);
		static void BindTexture(uint32_t slot, uint32_t id);
	};

	class Texture2DArray : public Texture
	{
	public:
		static Ref<Texture2DArray> Create(const TextureSpecification& specification, std::vector<Buffer> layers);
		
		virtual AssetType GetType() const override { return AssetType::Texture2DArray; }

		virtual uint8_t GetNumLayers() const = 0;

		virtual void SetData(Buffer data, uint8_t layer) = 0;
	};

	class TextureCubemap : public Texture
	{
	public:
		// First 6 elements of face data are uploaded to cubemap texture in the following face order: right, left, top, bottom, front, back. Empty buffers will be created to match face count if less than 6 buffers are provided
		static Ref<TextureCubemap> Create(const TextureSpecification& specification, std::vector<Buffer> faceData = std::vector<Buffer>(6));
	
		virtual AssetType GetType() const override { return AssetType::TextureCubemap; }

		virtual void SetData(Buffer data, CubemapFace face) = 0;
	};

	class TextureCubemapArray : public Texture
	{
	public:
		// First 6 elements of face data are uploaded to cubemap texture in the following face order: right, left, top, bottom, front, back. Empty buffers will be created to match face count if less than 6 buffers are provided
		static Ref<TextureCubemapArray> Create(const TextureSpecification& specification, std::vector<std::vector<Buffer>> faceDataLayers = std::vector<std::vector<Buffer>>(2, std::vector<Buffer>(6)));

		virtual AssetType GetType() const override { return AssetType::TextureCubemapArray; }

		virtual uint8_t GetNumLayers() const = 0;

		virtual void SetData(Buffer data, uint8_t layer, CubemapFace face) = 0;
	};

	class DeviceHandleTracker
	{
	public:
		static void UnregisterDeviceHandle(uint64_t deviceHandle)
		{
			auto it = s_DeviceToAssetHandleMap.find(deviceHandle);
			if (it != s_DeviceToAssetHandleMap.end())
			{
				s_AssetToDeviceHandleMap.erase(it->second);
				s_DeviceToAssetHandleMap.erase(it);
			}
		}
		static void RegisterTexture(Texture* textureInstance)
		{
			s_DeviceToAssetHandleMap[textureInstance->GetDeviceTextureHandle()] = textureInstance->Handle;
			s_AssetToDeviceHandleMap[textureInstance->Handle] = textureInstance->GetDeviceTextureHandle();
		}
		static AssetHandle DeviceHandleToAssetHandle(uint64_t deviceTextureHandle)
		{
			auto it = s_DeviceToAssetHandleMap.find(deviceTextureHandle);
			if (it != s_DeviceToAssetHandleMap.end())
			{
				return it->second;
			}
			return 0;
		}
		static uint64_t AssetHandleToDeviceHandle(AssetHandle assetHandle)
		{
			auto it = s_AssetToDeviceHandleMap.find(assetHandle);
			if (it != s_AssetToDeviceHandleMap.end())
			{
				return it->second;
			}
			return 0;
		}
	private:
		static std::unordered_map<uint64_t, AssetHandle> s_DeviceToAssetHandleMap;
		static std::unordered_map<AssetHandle, uint64_t> s_AssetToDeviceHandleMap;
	};
}