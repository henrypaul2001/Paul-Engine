#pragma once
#include "PaulEngine/Renderer/Asset/Texture.h"

typedef unsigned int GLenum;
namespace PaulEngine {

	namespace OpenGLTextureUtils
	{
		GLenum PEImageFormatToGLDataFormat(ImageFormat format);
		GLenum PEImageFormatToGLInternalFormat(ImageFormat format);
		GLenum ImageWrapToGLWrap(ImageWrap wrap);
		GLenum MinFilterToGLMinFilter(ImageMinFilter filter);
		GLenum MagFilterToGLMagFilter(ImageMagFilter filter);
	}

	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(const TextureSpecification& specification, Buffer data = Buffer());
		virtual ~OpenGLTexture2D();

		virtual const TextureSpecification& GetSpecification() const override { return m_Spec; }

		virtual void OnImport() override { UpdateDeviceHandle(); }

		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }
		virtual uint32_t GetRendererID() const override { return m_RendererID; }

		virtual void SetData(Buffer data) override;
		virtual Buffer GetData(uint8_t mipLevel) const override;

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
		void UpdateDeviceHandle();

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

		virtual void OnImport() override { UpdateDeviceHandle(); }

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
		void UpdateDeviceHandle();

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

		virtual void OnImport() override { UpdateDeviceHandle(); }

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
		void UpdateDeviceHandle();

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

		virtual void OnImport() override { UpdateDeviceHandle(); }

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
		void UpdateDeviceHandle();

		TextureSpecification m_Spec;

		uint8_t m_NumLayers;

		uint32_t m_Width, m_Height;
		uint32_t m_RendererID;

		GLenum m_InternalFormat;
		GLenum m_DataFormat;

		uint64_t m_DeviceHandle;
	};
}