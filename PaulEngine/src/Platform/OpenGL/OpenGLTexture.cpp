#include "pepch.h"
#include "OpenGLTexture.h"
#include <glad/gl.h>

namespace PaulEngine {
	namespace OpenGLTextureUtils
	{
		GLenum PEImageFormatToGLDataFormat(ImageFormat format) {
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
			case ImageFormat::R16F: return GL_RED;
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
		GLenum PEImageFormatToGLInternalFormat(ImageFormat format) {
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
			case ImageFormat::R16F: return GL_R16F;
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
		GLenum ImageWrapToGLWrap(ImageWrap wrap)
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
		GLenum MinFilterToGLMinFilter(ImageMinFilter filter)
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
		GLenum MagFilterToGLMagFilter(ImageMagFilter filter)
		{
			switch (filter)
			{
			case ImageMagFilter::NEAREST: return GL_NEAREST;
			case ImageMagFilter::LINEAR: return GL_LINEAR;
			}

			PE_CORE_ASSERT(false, "Undefined image mag filter translation");
			return 0;
		}
		GLenum PEImageFormatToGLPixelFormat(ImageFormat format)
		{
			PE_CORE_ASSERT(format != ImageFormat::None, "Undefined image format");
			switch (format)
			{
			case ImageFormat::Depth16: return GL_UNSIGNED_INT;
			case ImageFormat::Depth24: return GL_UNSIGNED_INT;
			case ImageFormat::Depth32: return GL_FLOAT;
			case ImageFormat::Depth24Stencil8: return GL_UNSIGNED_INT_24_8;
			case ImageFormat::RED_INTEGER: return GL_INT;
			case ImageFormat::R8: return GL_UNSIGNED_BYTE;
			case ImageFormat::RG8: return GL_UNSIGNED_BYTE;
			case ImageFormat::RGB8: return GL_UNSIGNED_BYTE;
			case ImageFormat::RGBA8: return GL_UNSIGNED_BYTE;
			case ImageFormat::R11FG11FB10F: return GL_UNSIGNED_INT_10F_11F_11F_REV;
			case ImageFormat::R16F: return GL_HALF_FLOAT;
			case ImageFormat::RG16F: return GL_HALF_FLOAT;
			case ImageFormat::RGB16F: return GL_HALF_FLOAT;
			case ImageFormat::RGBA16F: return GL_HALF_FLOAT;
			case ImageFormat::RG32F: return GL_FLOAT;
			case ImageFormat::RGB32F: return GL_FLOAT;
			case ImageFormat::RGBA32F: return GL_FLOAT;
			}

			PE_CORE_ASSERT(false, "Undefined image format translation");
			return 0;
		}
	}

#pragma region Texture2D
	OpenGLTexture2D::OpenGLTexture2D(const TextureSpecification& specification, Buffer data) : m_Spec(specification), m_Width(m_Spec.Width), m_Height(m_Spec.Height)
	{
		PE_PROFILE_FUNCTION();
		Generate(specification, data);
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		glDeleteTextures(1, &m_RendererID);
		DeviceHandleTracker::UnregisterDeviceHandle(m_DeviceHandle);
	}

	void OpenGLTexture2D::SetData(Buffer data)
	{
		PE_PROFILE_FUNCTION();
#ifdef PE_ENABLE_ASSERTS
		// TODO: This can't be right. A pixels size in bytes is not equal to the number of channels. Investigate why this seems to work and add new function for getting true byte size of pixel from format
		uint32_t sizeofpixel = PixelSize(m_Spec.Format);
		PE_CORE_ASSERT(data.Size() == m_Width * m_Height * sizeofpixel, "Data size must be entire texture!");
#endif
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, m_PixelType, data.m_Data);
		GenerateMipmaps();
	}

	Buffer OpenGLTexture2D::GetData(uint8_t mipLevel) const
	{
		uint32_t mipWidth = m_Width;
		uint32_t mipHeight = m_Height;

		if (mipLevel > 0)
		{
			// divide by power of 2
			mipWidth = std::max(1u, m_Width >> mipLevel);
			mipHeight = std::max(1u, m_Height >> mipLevel);
		}

		size_t bufferSize = mipWidth * mipHeight * PixelSize(m_Spec.Format);
		Buffer dataBuffer = Buffer(bufferSize);

		glGetTextureImage(m_RendererID, mipLevel, m_DataFormat, m_PixelType, bufferSize, dataBuffer.m_Data);

		return dataBuffer;
	}

	// TODO: Not sure if this is the right way to clear a texture by hardcoding GL_INT or GL_FLOAT based on parameter. It should be based on ImageFormat
	void OpenGLTexture2D::Clear(int value)
	{
		glClearTexImage(m_RendererID, 0, m_DataFormat, GL_INT, &value);
		GenerateMipmaps();
	}

	void OpenGLTexture2D::Clear(float value)
	{
		glClearTexImage(m_RendererID, 0, m_DataFormat, GL_FLOAT, &value);
		GenerateMipmaps();
	}

	void OpenGLTexture2D::GenerateMipmaps()
	{
		if (m_Spec.GenerateMips) { glGenerateTextureMipmap(m_RendererID); }
	}

	void OpenGLTexture2D::Resize(uint32_t width, uint32_t height)
	{
		TextureSpecification& spec = m_Spec;
		if (spec.Width != width || spec.Height != height)
		{
			spec.Width = width;
			spec.Height = height;
			glDeleteTextures(1, &m_RendererID);
			Generate(spec);
		}
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

	void OpenGLTexture2D::Generate(const TextureSpecification& specification, Buffer data)
	{
		m_Spec = specification;
		m_Width = specification.Width;
		m_Height = specification.Height;

		m_InternalFormat = OpenGLTextureUtils::PEImageFormatToGLInternalFormat(specification.Format);
		m_DataFormat = OpenGLTextureUtils::PEImageFormatToGLDataFormat(specification.Format);
		m_PixelType = OpenGLTextureUtils::PEImageFormatToGLPixelFormat(specification.Format);

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);

		// Calculate mipmap levels
		uint8_t levels = 1;
		if (m_Spec.GenerateMips)
		{
			levels = 1 + (uint8_t)std::floor(std::log2(std::max(m_Width, m_Height)));
		}

		glTextureStorage2D(m_RendererID, levels, m_InternalFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, OpenGLTextureUtils::MinFilterToGLMinFilter(m_Spec.MinFilter));
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, OpenGLTextureUtils::MagFilterToGLMagFilter(m_Spec.MagFilter));

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, OpenGLTextureUtils::ImageWrapToGLWrap(m_Spec.Wrap_S));
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, OpenGLTextureUtils::ImageWrapToGLWrap(m_Spec.Wrap_T));
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_R, OpenGLTextureUtils::ImageWrapToGLWrap(m_Spec.Wrap_R));

		if (IsIntegerFormat(m_Spec.Format))
		{
			glm::ivec4 intBorder = (glm::ivec4)m_Spec.Border;
			glTextureParameterIiv(m_RendererID, GL_TEXTURE_BORDER_COLOR, (const GLint*)&intBorder[0]);
		}
		else
		{
			glTextureParameterfv(m_RendererID, GL_TEXTURE_BORDER_COLOR, (const GLfloat*)&m_Spec.Border[0]);
		}

		UpdateDeviceHandle();

		if (data) { SetData(data); }
	}

	void OpenGLTexture2D::UpdateDeviceHandle()
	{
		DeviceHandleTracker::UnregisterDeviceHandle(m_DeviceHandle);
		m_DeviceHandle = glGetTextureHandleARB(m_RendererID);
		DeviceHandleTracker::RegisterTexture(this);
	}

#pragma endregion

#pragma region Texture2DArray
	OpenGLTexture2DArray::OpenGLTexture2DArray(const TextureSpecification& specification, std::vector<Buffer> layers) : m_Spec(specification), m_Width(m_Spec.Width), m_Height(m_Spec.Height)
	{
		PE_PROFILE_FUNCTION();
		Generate(specification, layers);
	}

	OpenGLTexture2DArray::~OpenGLTexture2DArray()
	{
		glDeleteTextures(1, &m_RendererID);
		DeviceHandleTracker::UnregisterDeviceHandle(m_DeviceHandle);
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
		uint32_t sizeofpixel = PixelSize(m_Spec.Format);
		PE_CORE_ASSERT(data.Size() == m_Width * m_Height * sizeofpixel, "Data size must be entire texture!");
#endif
		glTextureSubImage3D(m_RendererID, 0, 0, 0, layer, m_Width, m_Height, 1, m_DataFormat, m_PixelType, data.m_Data);
		GenerateMipmaps();
	}

	Buffer OpenGLTexture2DArray::GetData(uint8_t layer, uint8_t numLayers, uint8_t mipLevel) const
	{
		if (layer + numLayers > m_NumLayers)
		{
			PE_CORE_WARN("Attempting to retrieve excess layers in 2D texture array");
			uint8_t availableLayers = m_NumLayers - layer;
			PE_CORE_WARN("    {0} requested, {1} available", numLayers, availableLayers);
			numLayers = availableLayers;
		}

		if (numLayers == 0) { return Buffer(); }

		uint32_t mipWidth = m_Width;
		uint32_t mipHeight = m_Height;

		if (mipLevel > 0)
		{
			// divide by power of 2
			mipWidth = std::max(1u, m_Width >> mipLevel);
			mipHeight = std::max(1u, m_Height >> mipLevel);
		}

		size_t bufferSize = mipWidth * mipHeight * PixelSize(m_Spec.Format) * numLayers;
		Buffer dataBuffer = Buffer(bufferSize);

		glGetTextureSubImage(m_RendererID, mipLevel, 0, 0, layer, mipWidth, mipHeight, numLayers, m_DataFormat, m_PixelType, bufferSize, dataBuffer.m_Data);

		return dataBuffer;
	}

	void OpenGLTexture2DArray::Clear(int value)
	{
		glClearTexImage(m_RendererID, 0, m_DataFormat, GL_INT, &value);
		GenerateMipmaps();
	}

	void OpenGLTexture2DArray::Clear(float value)
	{
		glClearTexImage(m_RendererID, 0, m_DataFormat, GL_FLOAT, &value);
		GenerateMipmaps();
	}

	void OpenGLTexture2DArray::GenerateMipmaps()
	{
		if (m_Spec.GenerateMips) { glGenerateTextureMipmap(m_RendererID); }
	}

	void OpenGLTexture2DArray::Resize(uint32_t width, uint32_t height)
	{
		TextureSpecification& spec = m_Spec;
		if (spec.Width != width || spec.Height != height)
		{
			spec.Width = width;
			spec.Height = height;
			glDeleteTextures(1, &m_RendererID);
			Generate(spec);
		}
	}

	void OpenGLTexture2DArray::Bind(const uint32_t slot) const
	{
		glBindTextureUnit(slot, m_RendererID);
	}

	void OpenGLTexture2DArray::Generate(const TextureSpecification& specification, std::vector<Buffer> layers)
	{
		PE_CORE_ASSERT(layers.size() > 1, "Texture array needs at least 1 buffer as input. (Buffer can be empty)");

		m_Spec = specification;
		m_Width = specification.Width;
		m_Height = specification.Height;

		m_NumLayers = layers.size();

		m_InternalFormat = OpenGLTextureUtils::PEImageFormatToGLInternalFormat(specification.Format);
		m_DataFormat = OpenGLTextureUtils::PEImageFormatToGLDataFormat(specification.Format);
		m_PixelType = OpenGLTextureUtils::PEImageFormatToGLPixelFormat(specification.Format);

		glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_RendererID);

		// Calculate mipmap levels
		uint8_t levels = 1;
		if (m_Spec.GenerateMips)
		{
			levels = 1 + (uint8_t)std::floor(std::log2(std::max(m_Width, m_Height)));
		}

		glTextureStorage3D(m_RendererID, levels, m_InternalFormat, m_Width, m_Height, layers.size());

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, OpenGLTextureUtils::MinFilterToGLMinFilter(m_Spec.MinFilter));
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, OpenGLTextureUtils::MagFilterToGLMagFilter(m_Spec.MagFilter));

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, OpenGLTextureUtils::ImageWrapToGLWrap(m_Spec.Wrap_S));
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, OpenGLTextureUtils::ImageWrapToGLWrap(m_Spec.Wrap_T));
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_R, OpenGLTextureUtils::ImageWrapToGLWrap(m_Spec.Wrap_R));

		if (IsIntegerFormat(m_Spec.Format))
		{
			glm::ivec4 intBorder = (glm::ivec4)m_Spec.Border;
			glTextureParameterIiv(m_RendererID, GL_TEXTURE_BORDER_COLOR, (const GLint*)&intBorder[0]);
		}
		else
		{
			glTextureParameterfv(m_RendererID, GL_TEXTURE_BORDER_COLOR, (const GLfloat*)&m_Spec.Border[0]);
		}

		UpdateDeviceHandle();

		for (int i = 0; i < m_NumLayers; i++) {
			if (layers[i]) {
				SetData(layers[i], i);
			}
		}
	}

	void OpenGLTexture2DArray::UpdateDeviceHandle()
	{
		DeviceHandleTracker::UnregisterDeviceHandle(m_DeviceHandle);
		m_DeviceHandle = glGetTextureHandleARB(m_RendererID);
		DeviceHandleTracker::RegisterTexture(this);
	}

#pragma endregion

#pragma region TextureCubemap
	OpenGLTextureCubemap::OpenGLTextureCubemap(const TextureSpecification& specification, std::vector<Buffer> faceData) : m_Spec(specification), m_Width(m_Spec.Width), m_Height(m_Spec.Height)
	{
		PE_PROFILE_FUNCTION();
		Generate(specification, faceData);
	}

	OpenGLTextureCubemap::~OpenGLTextureCubemap()
	{
		glDeleteTextures(1, &m_RendererID);
		DeviceHandleTracker::UnregisterDeviceHandle(m_DeviceHandle);
	}

	void OpenGLTextureCubemap::SetData(Buffer data)
	{
		SetData(data, CubemapFace::POSITIVE_X);
	}

	void OpenGLTextureCubemap::SetData(Buffer data, CubemapFace face)
	{
		PE_PROFILE_FUNCTION();
#ifdef PE_ENABLE_ASSERTS
		uint32_t sizeofpixel = PixelSize(m_Spec.Format);
		PE_CORE_ASSERT(data.Size() == m_Width * m_Height * sizeofpixel, "Data size must be entire texture!");
#endif
		// cubemaps are represented internally as a 2D texture array, so access a specific face using a z-offset
		// cubemap arrays are represented as a 2D texture array with numLayers = 6 (faces) * cubemapArraySize. So z-offset for the "face" of cubemap index "i" would be: i * 6 + face;
		glTextureSubImage3D(m_RendererID, 0, 0, 0, (int)face, m_Width, m_Height, 1, m_DataFormat, m_PixelType, data.m_Data);
		GenerateMipmaps();
	}

	Buffer OpenGLTextureCubemap::GetData(CubemapFace face, uint8_t mipLevel) const
	{
		uint32_t mipWidth = m_Width;
		uint32_t mipHeight = m_Height;

		if (mipLevel > 0)
		{
			// divide by power of 2
			mipWidth = std::max(1u, m_Width >> mipLevel);
			mipHeight = std::max(1u, m_Height >> mipLevel);
		}

		size_t bufferSize = mipWidth * mipHeight * PixelSize(m_Spec.Format);
		Buffer dataBuffer = Buffer(bufferSize);

		glGetTextureSubImage(m_RendererID, mipLevel, 0, 0, (GLint)face, mipWidth, mipHeight, 1, m_DataFormat, m_PixelType, bufferSize, dataBuffer.m_Data);

		return dataBuffer;
	}

	void OpenGLTextureCubemap::Clear(int value)
	{
		glClearTexImage(m_RendererID, 0, m_DataFormat, GL_INT, &value);
		GenerateMipmaps();
	}

	void OpenGLTextureCubemap::Clear(float value)
	{
		glClearTexImage(m_RendererID, 0, m_DataFormat, GL_FLOAT, &value);
		GenerateMipmaps();
	}

	void OpenGLTextureCubemap::GenerateMipmaps()
	{
		if (m_Spec.GenerateMips) { glGenerateTextureMipmap(m_RendererID); }
	}

	void OpenGLTextureCubemap::Resize(uint32_t width, uint32_t height)
	{
		TextureSpecification& spec = m_Spec;
		if (spec.Width != width || spec.Height != height)
		{
			spec.Width = width;
			spec.Height = height;
			glDeleteTextures(1, &m_RendererID);
			Generate(spec);
		}
	}

	void OpenGLTextureCubemap::Bind(const uint32_t slot) const
	{
		glBindTextureUnit(slot, m_RendererID);
	}

	void OpenGLTextureCubemap::Generate(const TextureSpecification& specification, std::vector<Buffer> faceData)
	{
		m_Spec = specification;
		m_Width = specification.Width;
		m_Height = specification.Height;

		m_InternalFormat = OpenGLTextureUtils::PEImageFormatToGLInternalFormat(specification.Format);
		m_DataFormat = OpenGLTextureUtils::PEImageFormatToGLDataFormat(specification.Format);
		m_PixelType = OpenGLTextureUtils::PEImageFormatToGLPixelFormat(specification.Format);

		faceData.resize(6, Buffer());

		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_RendererID);

		// Calculate mipmap levels
		uint8_t levels = 1;
		if (m_Spec.GenerateMips)
		{
			levels = 1 + (uint8_t)std::floor(std::log2(std::max(m_Width, m_Height)));
		}

		glTextureStorage2D(m_RendererID, levels, m_InternalFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, OpenGLTextureUtils::MinFilterToGLMinFilter(m_Spec.MinFilter));
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, OpenGLTextureUtils::MagFilterToGLMagFilter(m_Spec.MagFilter));

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, OpenGLTextureUtils::ImageWrapToGLWrap(m_Spec.Wrap_S));
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, OpenGLTextureUtils::ImageWrapToGLWrap(m_Spec.Wrap_T));
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_R, OpenGLTextureUtils::ImageWrapToGLWrap(m_Spec.Wrap_R));

		if (IsIntegerFormat(m_Spec.Format))
		{
			glm::ivec4 intBorder = (glm::ivec4)m_Spec.Border;
			glTextureParameterIiv(m_RendererID, GL_TEXTURE_BORDER_COLOR, (const GLint*)&intBorder[0]);
		}
		else
		{
			glTextureParameterfv(m_RendererID, GL_TEXTURE_BORDER_COLOR, (const GLfloat*)&m_Spec.Border[0]);
		}

		UpdateDeviceHandle();

		for (uint8_t i = 0; i < 6; i++) {
			if (faceData[i])
			{
				SetData(faceData[i], (CubemapFace)i);
			}
		}
	}

	void OpenGLTextureCubemap::UpdateDeviceHandle()
	{
		DeviceHandleTracker::UnregisterDeviceHandle(m_DeviceHandle);
		m_DeviceHandle = glGetTextureHandleARB(m_RendererID);
		DeviceHandleTracker::RegisterTexture(this);
	}

#pragma endregion

#pragma region TextureCubemapArray
	OpenGLTextureCubemapArray::OpenGLTextureCubemapArray(const TextureSpecification& specification, std::vector<std::vector<Buffer>> faceDataLayers) : m_Spec(specification), m_Width(m_Spec.Width), m_Height(m_Spec.Height)
	{
		PE_PROFILE_FUNCTION();
		Generate(specification, faceDataLayers);
	}

	OpenGLTextureCubemapArray::~OpenGLTextureCubemapArray()
	{
		glDeleteTextures(1, &m_RendererID);
		DeviceHandleTracker::UnregisterDeviceHandle(m_DeviceHandle);
	}

	void OpenGLTextureCubemapArray::SetData(Buffer data)
	{
		SetData(data, 0, CubemapFace::POSITIVE_X);
	}

	void OpenGLTextureCubemapArray::SetData(Buffer data, uint8_t layer, CubemapFace face)
	{
		PE_PROFILE_FUNCTION();
		PE_CORE_ASSERT(layer < m_NumLayers, "Layer index out of range");
#ifdef PE_ENABLE_ASSERTS
		uint32_t sizeofpixel = PixelSize(m_Spec.Format);
		PE_CORE_ASSERT(data.Size() == m_Width * m_Height * sizeofpixel, "Data size must be entire texture!");
#endif
		glTextureSubImage3D(m_RendererID, 0, 0, 0, layer * 6 + (int)face, m_Width, m_Height, 1, m_DataFormat, m_PixelType, data.m_Data);
		GenerateMipmaps();
	}

	Buffer OpenGLTextureCubemapArray::GetData(CubemapFace face, uint8_t layer, uint8_t mipLevel) const
	{
		if (layer >= m_NumLayers)
		{
			PE_CORE_WARN("Attempting to retrieve excess layers in cubemap texture array");
			uint8_t availableLayers = m_NumLayers - 1;
			PE_CORE_WARN("    {0} requested, {1} available", layer, availableLayers);
			layer = availableLayers;
		}

		uint32_t mipWidth = m_Width;
		uint32_t mipHeight = m_Height;

		if (mipLevel > 0)
		{
			// divide by power of 2
			mipWidth = std::max(1u, m_Width >> mipLevel);
			mipHeight = std::max(1u, m_Height >> mipLevel);
		}

		size_t bufferSize = mipWidth * mipHeight * PixelSize(m_Spec.Format);
		Buffer dataBuffer = Buffer(bufferSize);

		glGetTextureSubImage(m_RendererID, mipLevel, 0, 0, layer * 6 + (GLint)face, mipWidth, mipHeight, 1, m_DataFormat, m_PixelType, bufferSize, dataBuffer.m_Data);

		return dataBuffer;
	}

	void OpenGLTextureCubemapArray::Clear(int value)
	{
		glClearTexImage(m_RendererID, 0, m_DataFormat, GL_INT, &value);
		GenerateMipmaps();
	}

	void OpenGLTextureCubemapArray::Clear(float value)
	{
		glClearTexImage(m_RendererID, 0, m_DataFormat, GL_FLOAT, &value);
		GenerateMipmaps();
	}

	void OpenGLTextureCubemapArray::GenerateMipmaps()
	{
		if (m_Spec.GenerateMips) { glGenerateTextureMipmap(m_RendererID); }
	}

	void OpenGLTextureCubemapArray::Resize(uint32_t width, uint32_t height)
	{
		TextureSpecification& spec = m_Spec;
		if (spec.Width != width || spec.Height != height)
		{
			spec.Width = width;
			spec.Height = height;
			glDeleteTextures(1, &m_RendererID);
			Generate(spec);
		}
	}

	void OpenGLTextureCubemapArray::Bind(const uint32_t slot) const
	{
		glBindTextureUnit(slot, m_RendererID);
	}

	void OpenGLTextureCubemapArray::Generate(const TextureSpecification& specification, std::vector<std::vector<Buffer>> faceDataLayers)
	{
		PE_CORE_ASSERT(faceDataLayers.size() > 0, "Must provide an initial size for cubemap array");

		m_Spec = specification;
		m_Width = specification.Width;
		m_Height = specification.Height;

		m_InternalFormat = OpenGLTextureUtils::PEImageFormatToGLInternalFormat(specification.Format);
		m_DataFormat = OpenGLTextureUtils::PEImageFormatToGLDataFormat(specification.Format);
		m_PixelType = OpenGLTextureUtils::PEImageFormatToGLPixelFormat(specification.Format);

		m_NumLayers = faceDataLayers.size();

		glCreateTextures(GL_TEXTURE_CUBE_MAP_ARRAY, 1, &m_RendererID);

		// Calculate mipmap levels
		uint8_t levels = 1;
		if (m_Spec.GenerateMips)
		{
			levels = 1 + (uint8_t)std::floor(std::log2(std::max(m_Width, m_Height)));
		}

		glTextureStorage3D(m_RendererID, levels, m_InternalFormat, m_Width, m_Height, faceDataLayers.size() * 6);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, OpenGLTextureUtils::MinFilterToGLMinFilter(m_Spec.MinFilter));
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, OpenGLTextureUtils::MagFilterToGLMagFilter(m_Spec.MagFilter));

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, OpenGLTextureUtils::ImageWrapToGLWrap(m_Spec.Wrap_S));
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, OpenGLTextureUtils::ImageWrapToGLWrap(m_Spec.Wrap_T));
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_R, OpenGLTextureUtils::ImageWrapToGLWrap(m_Spec.Wrap_R));

		if (IsIntegerFormat(m_Spec.Format))
		{
			glm::ivec4 intBorder = (glm::ivec4)m_Spec.Border;
			glTextureParameterIiv(m_RendererID, GL_TEXTURE_BORDER_COLOR, (const GLint*)&intBorder[0]);
		}
		else
		{
			glTextureParameterfv(m_RendererID, GL_TEXTURE_BORDER_COLOR, (const GLfloat*)&m_Spec.Border[0]);
		}

		UpdateDeviceHandle();

		for (uint8_t layer = 0; layer < faceDataLayers.size(); layer++)
		{
			faceDataLayers[layer].resize(6, Buffer());
			for (uint8_t face = 0; face < 6; face++) {
				if (faceDataLayers[layer][face])
				{
					SetData(faceDataLayers[layer][face], layer, (CubemapFace)face);
				}
			}
		}
	}

	void OpenGLTextureCubemapArray::UpdateDeviceHandle()
	{
		DeviceHandleTracker::UnregisterDeviceHandle(m_DeviceHandle);
		m_DeviceHandle = glGetTextureHandleARB(m_RendererID);
		DeviceHandleTracker::RegisterTexture(this);
	}

#pragma endregion
}