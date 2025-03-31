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

		virtual AssetHandle GetAtlasHandle() const override { return m_AtlasHandle; }
		virtual void SetAtlasHandle(AssetHandle atlasHandle) override { m_AtlasHandle = atlasHandle; }

	private:
		TextureSpecification m_Spec;

		uint32_t m_Width, m_Height;
		uint32_t m_RendererID;

		GLenum m_InternalFormat;
		GLenum m_DataFormat;

		AssetHandle m_AtlasHandle = 0;
	};
}