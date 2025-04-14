#include "pepch.h"
#include "OpenGLTexture.h"

namespace PaulEngine {
	namespace OpenGLTextureUtils
	{
		static GLenum PEImageFormatToGLDataFormat(ImageFormat format) {
			switch (format) {
				case ImageFormat::R8: return GL_RED;
				case ImageFormat::RG8: return GL_RG;
				case ImageFormat::RGB8: return GL_RGB;
				case ImageFormat::RGBA8: return GL_RGBA;
				case ImageFormat::RGBA32F: return GL_RGBA;
			}

			PE_CORE_ASSERT(false, "Undefined image format translation");
			return 0;
		}

		static GLenum PEImageFormatToGLInternalFormat(ImageFormat format) {
			switch (format) {
				case ImageFormat::R8: return GL_R8;
				case ImageFormat::RG8: return GL_RG8;
				case ImageFormat::RGB8: return GL_RGB8;
				case ImageFormat::RGBA8: return GL_RGBA8;
				case ImageFormat::RGBA32F: return GL_RGBA32F;
			}

			PE_CORE_ASSERT(false, "Undefined image format translation");
			return 0;
		}
	}

#pragma region Texture2D
	OpenGLTexture2D::OpenGLTexture2D(const TextureSpecification& specification, Buffer data) : m_Spec(specification), m_Width(m_Spec.Width), m_Height(m_Spec.Height)
	{
		PE_PROFILE_FUNCTION();
		m_InternalFormat = OpenGLTextureUtils::PEImageFormatToGLInternalFormat(specification.Format);
		m_DataFormat = OpenGLTextureUtils::PEImageFormatToGLDataFormat(specification.Format);

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

		if (data) { SetData(data); }
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		glDeleteTextures(1, &m_RendererID);
	}

	void OpenGLTexture2D::SetData(Buffer data)
	{
		PE_PROFILE_FUNCTION();
#ifdef PE_ENABLE_ASSERTS
		uint32_t sizeofpixel = 0;
		switch (m_DataFormat) {
		case GL_RGBA:
			sizeofpixel = 4;
			break;
		case GL_RGB:
			sizeofpixel = 3;
			break;
		case GL_RG:
			sizeofpixel = 2;
			break;
		case GL_RED:
			sizeofpixel = 1;
			break;
		}
		PE_CORE_ASSERT(data.Size() == m_Width * m_Height * sizeofpixel, "Data size must be entire texture!");
#endif
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data.m_Data);
	}

	void OpenGLTexture2D::Bind(const uint32_t slot) const
	{
		glBindTextureUnit(slot, m_RendererID);
	}
#pragma endregion

#pragma region Texture2DArray
	OpenGLTexture2DArray::OpenGLTexture2DArray(const TextureSpecification& specification, std::vector<Buffer> layers) : m_Spec(specification), m_Width(m_Spec.Width), m_Height(m_Spec.Height)
	{
		PE_PROFILE_FUNCTION();
		PE_CORE_ASSERT(layers.size() > 1, "Texture array needs at least 1 buffer as input. (Buffer can be empty)");

		m_NumLayers = layers.size();

		m_InternalFormat = OpenGLTextureUtils::PEImageFormatToGLInternalFormat(specification.Format);
		m_DataFormat = OpenGLTextureUtils::PEImageFormatToGLDataFormat(specification.Format);

		glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_RendererID);
		glTextureStorage3D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height, layers.size());

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

		for (int i = 0; i < m_NumLayers; i++) {
			if (layers[i]) {
				SetData(layers[i], i);
			}
		}
	}

	OpenGLTexture2DArray::~OpenGLTexture2DArray()
	{
		glDeleteTextures(1, &m_RendererID);
	}

	void OpenGLTexture2DArray::SetData(Buffer data)
	{
		SetData(data, 0);
	}

	void OpenGLTexture2DArray::SetData(Buffer data, uint8_t layer)
	{
		PE_PROFILE_FUNCTION();
		PE_CORE_ASSERT(layer < m_NumLayers, "Layer index out of range");
#ifdef PE_ENABLE_ASSERTS
		uint32_t sizeofpixel = 0;
		switch (m_DataFormat) {
		case GL_RGBA:
			sizeofpixel = 4;
			break;
		case GL_RGB:
			sizeofpixel = 3;
			break;
		case GL_RG:
			sizeofpixel = 2;
			break;
		case GL_RED:
			sizeofpixel = 1;
			break;
		}
		PE_CORE_ASSERT(data.Size() == m_Width * m_Height * sizeofpixel, "Data size must be entire texture!");
#endif
		glTextureSubImage3D(m_RendererID, 0, 0, 0, layer, m_Width, m_Height, 1, m_DataFormat, GL_UNSIGNED_BYTE, data.m_Data);
	}

	void OpenGLTexture2DArray::Bind(const uint32_t slot) const
	{
		glBindTextureUnit(slot, m_RendererID);
	}
#pragma endregion
}