#include "pepch.h"
#include "OpenGLTexture.h"

namespace PaulEngine {
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
		uint32_t sizeofpixel = NumChannels(m_Spec.Format);
		PE_CORE_ASSERT(data.Size() == m_Width * m_Height * sizeofpixel, "Data size must be entire texture!");
#endif
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data.m_Data);
	}

	void OpenGLTexture2D::Clear(int value)
	{
		glClearTexImage(m_RendererID, 0, OpenGLTextureUtils::PEImageFormatToGLDataFormat(m_Spec.Format), GL_INT, &value);
	}

	void OpenGLTexture2D::Clear(float value)
	{
		glClearTexImage(m_RendererID, 0, OpenGLTextureUtils::PEImageFormatToGLDataFormat(m_Spec.Format), GL_FLOAT, &value);
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
		uint32_t sizeofpixel = NumChannels(m_Spec.Format);
		PE_CORE_ASSERT(data.Size() == m_Width * m_Height * sizeofpixel, "Data size must be entire texture!");
#endif
		glTextureSubImage3D(m_RendererID, 0, 0, 0, layer, m_Width, m_Height, 1, m_DataFormat, GL_UNSIGNED_BYTE, data.m_Data);
	}

	void OpenGLTexture2DArray::Clear(int value)
	{
		glClearTexImage(m_RendererID, 0, OpenGLTextureUtils::PEImageFormatToGLDataFormat(m_Spec.Format), GL_INT, &value);
	}

	void OpenGLTexture2DArray::Clear(float value)
	{
		glClearTexImage(m_RendererID, 0, OpenGLTextureUtils::PEImageFormatToGLDataFormat(m_Spec.Format), GL_FLOAT, &value);
	}

	void OpenGLTexture2DArray::Bind(const uint32_t slot) const
	{
		glBindTextureUnit(slot, m_RendererID);
	}
#pragma endregion

#pragma region TextureCubemap
	OpenGLTextureCubemap::OpenGLTextureCubemap(const TextureSpecification& specification, std::vector<Buffer> faceData) : m_Spec(specification), m_Width(m_Spec.Width), m_Height(m_Spec.Height)
	{
		PE_PROFILE_FUNCTION();
		m_InternalFormat = OpenGLTextureUtils::PEImageFormatToGLInternalFormat(specification.Format);
		m_DataFormat = OpenGLTextureUtils::PEImageFormatToGLDataFormat(specification.Format);

		faceData.resize(6, Buffer());

		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, OpenGLTextureUtils::MinFilterToGLMinFilter(m_Spec.MinFilter));
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, OpenGLTextureUtils::MagFilterToGLMagFilter(m_Spec.MagFilter));

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, OpenGLTextureUtils::ImageWrapToGLWrap(m_Spec.Wrap_S));
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, OpenGLTextureUtils::ImageWrapToGLWrap(m_Spec.Wrap_T));
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_R, OpenGLTextureUtils::ImageWrapToGLWrap(m_Spec.Wrap_R));

		glTextureParameterfv(m_RendererID, GL_TEXTURE_BORDER_COLOR, &m_Spec.Border[0]);

		for (uint8_t i = 0; i < 6; i++) {
			if (faceData[i])
			{
				SetData(faceData[i], (CubemapFace)i);
			}
		}
	}

	OpenGLTextureCubemap::~OpenGLTextureCubemap()
	{
		glDeleteTextures(1, &m_RendererID);
	}

	void OpenGLTextureCubemap::SetData(Buffer data)
	{
		SetData(data, CubemapFace::POSITIVE_X);
	}

	void OpenGLTextureCubemap::SetData(Buffer data, CubemapFace face)
	{
		PE_PROFILE_FUNCTION();
#ifdef PE_ENABLE_ASSERTS
		uint32_t sizeofpixel = NumChannels(m_Spec.Format);
		PE_CORE_ASSERT(data.Size() == m_Width * m_Height * sizeofpixel, "Data size must be entire texture!");
#endif
		// cubemaps are represented internally as a 2D texture array, so access a specific face using a z-offset
		// cubemap arrays are represented as a 2D texture array with numLayers = 6 (faces) * cubemapArraySize. So z-offset for the "face" of cubemap index "i" would be: i * 6 + face;
		glTextureSubImage3D(m_RendererID, 0, 0, 0, (int)face, m_Width, m_Height, 1, m_DataFormat, GL_UNSIGNED_BYTE, data.m_Data);
	}

	void OpenGLTextureCubemap::Clear(int value)
	{
		glClearTexImage(m_RendererID, 0, OpenGLTextureUtils::PEImageFormatToGLDataFormat(m_Spec.Format), GL_INT, &value);
	}

	void OpenGLTextureCubemap::Clear(float value)
	{
		glClearTexImage(m_RendererID, 0, OpenGLTextureUtils::PEImageFormatToGLDataFormat(m_Spec.Format), GL_FLOAT, &value);
	}

	void OpenGLTextureCubemap::Bind(const uint32_t slot) const
	{
		glBindTextureUnit(slot, m_RendererID);
	}
#pragma endregion
}