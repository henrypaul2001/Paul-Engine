#pragma once
#include "PaulEngine/Renderer/Texture.h"
#include <glad/glad.h>

namespace PaulEngine {
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

		virtual void Bind(const uint32_t slot = 0) const override;
	
		virtual bool operator==(const Texture& other) const override {
			return m_RendererID == ((OpenGLTexture2D&)other).m_RendererID;
		}

	private:
		TextureSpecification m_Spec;

		uint32_t m_Width, m_Height;
		uint32_t m_RendererID;

		GLenum m_InternalFormat;
		GLenum m_DataFormat;
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

		virtual void Bind(const uint32_t slot = 0) const override;

		virtual bool operator==(const Texture& other) const override {
			return m_RendererID == ((OpenGLTexture2DArray&)other).m_RendererID;
		}

	private:
		TextureSpecification m_Spec;

		uint8_t m_NumLayers;
	
		uint32_t m_Width, m_Height;
		uint32_t m_RendererID;

		GLenum m_InternalFormat;
		GLenum m_DataFormat;
	};
}