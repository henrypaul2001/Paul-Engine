#include "pepch.h"
#include "OpenGLTexture.h"

namespace PaulEngine {
	namespace OpenGLTextureUtils
	{
		static GLenum PEImageFormatToGLDataFormat(ImageFormat format) {
			switch (format) {
				case ImageFormat::DEPTH: return GL_DEPTH_COMPONENT;
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
				case ImageFormat::DEPTH: return GL_DEPTH_COMPONENT;
				case ImageFormat::R8: return GL_R8;
				case ImageFormat::RG8: return GL_RG8;
				case ImageFormat::RGB8: return GL_RGB8;
				case ImageFormat::RGBA8: return GL_RGBA8;
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

		static uint32_t GetPixelSize(GLenum dataFormat) {
			switch (dataFormat) {
				case GL_RGBA:
					return 4;
				case GL_RGB:
					return 3;
				case GL_RG:
					return 2;
				case GL_RED:
					return 1;
				case GL_DEPTH_COMPONENT:
					return 1;
			}

			PE_CORE_ASSERT(false, "Undefined data format translation");
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

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, OpenGLTextureUtils::MinFilterToGLMinFilter(m_Spec.MinFilter));
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, OpenGLTextureUtils::MagFilterToGLMagFilter(m_Spec.MagFilter));

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, OpenGLTextureUtils::ImageWrapToGLWrap(m_Spec.Wrap_S));
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, OpenGLTextureUtils::ImageWrapToGLWrap(m_Spec.Wrap_T));
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_R, OpenGLTextureUtils::ImageWrapToGLWrap(m_Spec.Wrap_R));

		glTextureParameterfv(m_RendererID, GL_TEXTURE_BORDER_COLOR, &m_Spec.Border[0]);

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
		uint32_t sizeofpixel = OpenGLTextureUtils::GetPixelSize(m_DataFormat);
		PE_CORE_ASSERT(data.Size() == m_Width * m_Height * sizeofpixel, "Data size must be entire texture!");
#endif
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data.m_Data);
	}

	void OpenGLTexture2D::Bind(const uint32_t slot) const
	{
		glBindTextureUnit(slot, m_RendererID);
	}

	GLenum OpenGLTexture2D::TextureTarget(bool multisampled)
	{
		return multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
	}

	void OpenGLTexture2D::CreateTextures(bool multisampled, uint32_t* out_ID, uint32_t count)
	{
		glCreateTextures(TextureTarget(multisampled), count, out_ID);
	}

	void OpenGLTexture2D::BindTexture(uint32_t slot, uint32_t id)
	{
		glBindTextureUnit(slot, id);
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

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, OpenGLTextureUtils::MinFilterToGLMinFilter(m_Spec.MinFilter));
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, OpenGLTextureUtils::MagFilterToGLMagFilter(m_Spec.MagFilter));

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, OpenGLTextureUtils::ImageWrapToGLWrap(m_Spec.Wrap_S));
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, OpenGLTextureUtils::ImageWrapToGLWrap(m_Spec.Wrap_T));
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_R, OpenGLTextureUtils::ImageWrapToGLWrap(m_Spec.Wrap_R));

		glTextureParameterfv(m_RendererID, GL_TEXTURE_BORDER_COLOR, &m_Spec.Border[0]);

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
		uint32_t sizeofpixel = OpenGLTextureUtils::GetPixelSize(m_DataFormat);
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