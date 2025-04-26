#pragma once
#include <string>
#include "PaulEngine/Asset/Asset.h"
#include "PaulEngine/Core/Core.h"
#include "PaulEngine/Core/Buffer.h"

namespace PaulEngine {
	enum class ImageFormat
	{
		None = 0,
		R8,
		RG8,
		RGB8,
		RGBA8,
		RGBA32F
	};

	struct TextureSpecification
	{
		uint32_t Width = 1;
		uint32_t Height = 1;
		ImageFormat Format = ImageFormat::RGBA8;
		bool GenerateMips = true;
	};

	class Texture : public Asset
	{
	public:
		virtual ~Texture() = default;

		virtual const TextureSpecification& GetSpecification() const = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetRendererID() const = 0;

		virtual void SetData(Buffer data) = 0;

		virtual void Bind(const uint32_t slot = 0) const = 0;

		virtual bool operator==(const Texture& other) const = 0;
	};

	class Texture2D : public Texture
	{
	public:
		static Ref<Texture2D> Create(const TextureSpecification& specification, Buffer data = Buffer());

		static AssetType GetStaticType() { return AssetType::Texture2D; }
		virtual AssetType GetType() const { return GetStaticType(); }

		static void CreateTextures(bool multisampled, uint32_t* out_ID, uint32_t count);
		static void BindTexture(uint32_t slot, uint32_t id);

		//static void CreateTextures(bool multisampled, uint32_t* out_ID, uint32_t count) {
		//	glCreateTextures(TextureTarget(multisampled), count, out_ID);
		//}
		//
		//static void BindTexture(bool multisampled, uint32_t id) {
		//	glBindTexture(TextureTarget(multisampled), id);
		//}
	};

	class Texture2DArray : public Texture
	{
	public:
		static Ref<Texture2DArray> Create(const TextureSpecification& specification, std::vector<Buffer> layers);
		
		virtual AssetType GetType() const { return AssetType::Texture2DArray; }

		virtual uint8_t GetNumLayers() const = 0;

		virtual void SetData(Buffer data, uint8_t layer) = 0;
	};
}