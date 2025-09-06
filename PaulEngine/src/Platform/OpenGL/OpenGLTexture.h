#pragma once
#include "PaulEngine/Renderer/Asset/Texture.h"
#include <glad/gl.h>

namespace PaulEngine {
	namespace OpenGLTextureUtils
	{
		static GLenum PEImageFormatToGLDataFormat(ImageFormat format) {
			switch (format) {
				case ImageFormat::Depth16: return GL_DEPTH_COMPONENT;
				case ImageFormat::Depth24: return GL_DEPTH_COMPONENT;
				case ImageFormat::Depth32: return GL_DEPTH_COMPONENT;
				case ImageFormat::Depth24Stencil8: return GL_DEPTH_STENCIL;
				case ImageFormat::RED_INTEGER: return GL_RED_INTEGER;
				case ImageFormat::R8: return GL_RED;
				case ImageFormat::RG8: return GL_RG;
				case ImageFormat::RGB8: return GL_RGB;
				case ImageFormat::RGBA8: return GL_RGBA;
				case ImageFormat::R11FG11FB10F: return GL_RGB;
				case ImageFormat::RG16F: return GL_RG;
				case ImageFormat::RGB16F: return GL_RGB;
				case ImageFormat::RGBA16F: return GL_RGBA;
				case ImageFormat::RG32F: return GL_RG;
				case ImageFormat::RGB32F: return GL_RGB;
				case ImageFormat::RGBA32F: return GL_RGBA;
			}

			PE_CORE_ASSERT(false, "Undefined image format translation");
			return 0;
		}

		static GLenum PEImageFormatToGLInternalFormat(ImageFormat format) {
			switch (format) {
				case ImageFormat::Depth16: return GL_DEPTH_COMPONENT16;
				case ImageFormat::Depth24: return GL_DEPTH_COMPONENT24;
				case ImageFormat::Depth32: return GL_DEPTH_COMPONENT32;
				case ImageFormat::Depth24Stencil8: return GL_DEPTH24_STENCIL8;
				case ImageFormat::RED_INTEGER: return GL_R32I;
				case ImageFormat::R8: return GL_R8;
				case ImageFormat::RG8: return GL_RG8;
				case ImageFormat::RGB8: return GL_RGB8;
				case ImageFormat::RGBA8: return GL_RGBA8;
				case ImageFormat::R11FG11FB10F: return GL_R11F_G11F_B10F;
				case ImageFormat::RG16F: return GL_RG16F;
				case ImageFormat::RGB16F: return GL_RGB16F;
				case ImageFormat::RGBA16F: return GL_RGBA16F;
				case ImageFormat::RG32F: return GL_RG32F;
				case ImageFormat::RGB32F: return GL_RGB32F;
				case ImageFormat::RGBA32F: return GL_RGBA32F;
			}

			PE_CORE_ASSERT(false, "Undefined image format translation");
			return 0;
		}

		static GLenum ImageWrapToGLWrap(ImageWrap wrap)
		{
			switch (wrap)
			{
				case ImageWrap::CLAMP_TO_EDGE: return GL_CLAMP_TO_EDGE;
				case ImageWrap::CLAMP_TO_BORDER: return GL_CLAMP_TO_BORDER;
				case ImageWrap::MIRRORED_REPEAT: return GL_MIRRORED_REPEAT;
				case ImageWrap::REPEAT: return GL_REPEAT;
				case ImageWrap::MIRROR_CLAMP_TO_EDGE: return GL_MIRROR_CLAMP_TO_EDGE;
			}

			PE_CORE_ASSERT(false, "Undefined image wrap translation");
			return 0;
		}

		static GLenum MinFilterToGLMinFilter(ImageMinFilter filter)
		{
			switch (filter)
			{
				case ImageMinFilter::NEAREST: return GL_NEAREST;
				case ImageMinFilter::LINEAR: return GL_LINEAR;
				case ImageMinFilter::NEAREST_MIPMAP_NEAREST: return GL_NEAREST_MIPMAP_NEAREST;
				case ImageMinFilter::LINEAR_MIPMAP_NEAREST: return GL_LINEAR_MIPMAP_NEAREST;
				case ImageMinFilter::NEAREST_MIPMAP_LINEAR: return GL_NEAREST_MIPMAP_LINEAR;
				case ImageMinFilter::LINEAR_MIPMAP_LINEAR: return GL_LINEAR_MIPMAP_LINEAR;
			}

			PE_CORE_ASSERT(false, "Undefined image min filter translation");
			return 0;
		}

		static GLenum MagFilterToGLMagFilter(ImageMagFilter filter)
		{
			switch (filter)
			{
				case ImageMagFilter::NEAREST: return GL_NEAREST;
				case ImageMagFilter::LINEAR: return GL_LINEAR;
			}

			PE_CORE_ASSERT(false, "Undefined image mag filter translation");
			return 0;
		}
	}

	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(const TextureSpecification& specification, Buffer data = Buffer());
		virtual ~OpenGLTexture2D();

		virtual const TextureSpecification& GetSpecification() const override { return m_Spec; }

		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }
		virtual uint32_t GetRendererID() const override { return m_RendererID; }

		virtual void SetData(Buffer data) override;

		virtual void Clear(int value) override;
		virtual void Clear(float value) override;

		virtual void GenerateMipmaps() override;
		virtual void Resize(uint32_t width, uint32_t height) override;

		virtual void Bind(const uint32_t slot = 0) const override;
		virtual uint64_t GetDeviceTextureHandle() const override { return m_DeviceHandle; }
	
		static GLenum TextureTarget(bool multisampled);
		static void CreateTextures(bool multisampled, uint32_t* out_ID, uint32_t count);
		static void BindTexture(uint32_t slot, uint32_t id);

		virtual bool operator==(const Texture& other) const override {
			return m_RendererID == ((OpenGLTexture2D&)other).m_RendererID;
		}

	private:
		void Generate(const TextureSpecification& specification, Buffer data = Buffer());

		TextureSpecification m_Spec;

		uint32_t m_Width, m_Height;
		uint32_t m_RendererID;

		GLenum m_InternalFormat;
		GLenum m_DataFormat;

		uint64_t m_DeviceHandle;
	};

	class OpenGLTexture2DArray : public Texture2DArray
	{
	public:
		OpenGLTexture2DArray(const TextureSpecification& specification, std::vector<Buffer> layers);
		virtual ~OpenGLTexture2DArray();

		virtual const TextureSpecification& GetSpecification() const override { return m_Spec; }

		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }
		virtual uint32_t GetRendererID() const override { return m_RendererID; }
		
		virtual uint8_t GetNumLayers() const override { return m_NumLayers; }

		virtual void SetData(Buffer data) override;
		virtual void SetData(Buffer data, uint8_t layer) override;

		virtual void Clear(int value) override;
		virtual void Clear(float value) override;

		virtual void GenerateMipmaps() override;
		virtual void Resize(uint32_t width, uint32_t height) override;

		virtual void Bind(const uint32_t slot = 0) const override;
		virtual uint64_t GetDeviceTextureHandle() const override { return m_DeviceHandle; }

		virtual bool operator==(const Texture& other) const override {
			return m_RendererID == ((OpenGLTexture2DArray&)other).m_RendererID;
		}

	private:
		void Generate(const TextureSpecification& specification) { Generate(specification, std::vector<Buffer>(m_NumLayers)); }
		void Generate(const TextureSpecification& specification, std::vector<Buffer> layers);

		TextureSpecification m_Spec;

		uint8_t m_NumLayers;
	
		uint32_t m_Width, m_Height;
		uint32_t m_RendererID;

		GLenum m_InternalFormat;
		GLenum m_DataFormat;

		uint64_t m_DeviceHandle;
	};

	class OpenGLTextureCubemap : public TextureCubemap
	{
	public:
		OpenGLTextureCubemap(const TextureSpecification& specification, std::vector<Buffer> faceData);
		virtual ~OpenGLTextureCubemap();

		virtual const TextureSpecification& GetSpecification() const override { return m_Spec; }

		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }
		virtual uint32_t GetRendererID() const override { return m_RendererID; }

		virtual void SetData(Buffer data) override;
		virtual void SetData(Buffer data, CubemapFace face) override;

		virtual void Clear(int value) override;
		virtual void Clear(float value) override;

		virtual void GenerateMipmaps() override;
		virtual void Resize(uint32_t width, uint32_t height) override;

		virtual void Bind(const uint32_t slot = 0) const override;
		virtual uint64_t GetDeviceTextureHandle() const override { return m_DeviceHandle; }

		virtual bool operator==(const Texture& other) const override {
			return m_RendererID == ((OpenGLTextureCubemap&)other).m_RendererID;
		}

	private:
		void Generate(const TextureSpecification& specification, std::vector<Buffer> faceData = std::vector<Buffer>(6));

		TextureSpecification m_Spec;

		uint32_t m_Width, m_Height;
		uint32_t m_RendererID;

		GLenum m_InternalFormat;
		GLenum m_DataFormat;

		uint64_t m_DeviceHandle;
	};

	class OpenGLTextureCubemapArray : public TextureCubemapArray
	{
	public:
		OpenGLTextureCubemapArray(const TextureSpecification& specification, std::vector<std::vector<Buffer>> faceDataLayers);
		virtual ~OpenGLTextureCubemapArray();

		virtual const TextureSpecification& GetSpecification() const override { return m_Spec; }

		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }
		virtual uint32_t GetRendererID() const override { return m_RendererID; }

		virtual uint8_t GetNumLayers() const override { return m_NumLayers; }

		virtual void SetData(Buffer data) override;
		virtual void SetData(Buffer data, uint8_t layer, CubemapFace face) override;

		virtual void Clear(int value) override;
		virtual void Clear(float value) override;

		virtual void GenerateMipmaps() override;
		virtual void Resize(uint32_t width, uint32_t height) override;

		virtual void Bind(const uint32_t slot = 0) const override;
		virtual uint64_t GetDeviceTextureHandle() const override { return m_DeviceHandle; }

		virtual bool operator==(const Texture& other) const override {
			return m_RendererID == ((OpenGLTextureCubemapArray&)other).m_RendererID;
		}

	private:
		void Generate(const TextureSpecification& specification) { Generate(specification, std::vector<std::vector<Buffer>>(m_NumLayers, std::vector<Buffer>(6))); }
		void Generate(const TextureSpecification& specification, std::vector<std::vector<Buffer>> faceDataLayers);

		TextureSpecification m_Spec;

		uint8_t m_NumLayers;

		uint32_t m_Width, m_Height;
		uint32_t m_RendererID;

		GLenum m_InternalFormat;
		GLenum m_DataFormat;

		uint64_t m_DeviceHandle;
	};
}