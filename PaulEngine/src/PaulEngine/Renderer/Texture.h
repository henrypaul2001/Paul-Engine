#pragma once
#include "PaulEngine/Asset/Asset.h"
#include "PaulEngine/Core/Core.h"
#include "PaulEngine/Core/Buffer.h"

#include <glm/glm.hpp>
#include <string>
namespace PaulEngine {
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
		RGBA32F
	};

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
		ImageMinFilter MinFilter = ImageMinFilter::LINEAR;
		ImageMagFilter MagFilter = ImageMagFilter::LINEAR;
		ImageWrap Wrap_S = ImageWrap::REPEAT;
		ImageWrap Wrap_T = ImageWrap::REPEAT;
		ImageWrap Wrap_R = ImageWrap::REPEAT;
		glm::vec4 Border = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		bool GenerateMips = true;
	};

	static uint32_t NumChannels(ImageFormat format)
	{
		switch (format)
		{
			case ImageFormat::Depth16: return 1;
			case ImageFormat::Depth24: return 1;
			case ImageFormat::Depth32: return 1;
			case ImageFormat::Depth24Stencil8: return 2;
			case ImageFormat::RED_INTEGER: return 1;
			case ImageFormat::R8: return 1;
			case ImageFormat::RG8: return 2;
			case ImageFormat::RGB8: return 3;
			case ImageFormat::RGBA8: return 4;
			case ImageFormat::RGBA32F: return 4;
		}

		PE_CORE_ASSERT(false, "Undefined data format translation");
		return 0;
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

		virtual void Bind(const uint32_t slot = 0) const = 0;

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

		virtual void SetData(Buffer data, uint8_t face) = 0;
	};
}