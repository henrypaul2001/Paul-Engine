#pragma once
#include "PaulEngine/Core/Core.h"
#include "Texture.h"
#include <glm/glm.hpp>
namespace PaulEngine {
	enum class FramebufferTextureFormat
	{
		None = 0,

		// Colour
		RGBA8,
		RED_INTEGER,

		// Depth / stencil
		DEPTH16,
		DEPTH24,
		DEPTH32,
		DEPTH24STENCIL8,

		// Defaults
		DepthStencil = DEPTH24STENCIL8,
		Depth = DEPTH32
	};

	struct FramebufferTextureSpecification
	{
		FramebufferTextureSpecification() = default;
		FramebufferTextureSpecification(FramebufferTextureFormat format) : TextureFormat(format) {}

		FramebufferTextureFormat TextureFormat = FramebufferTextureFormat::None;
		ImageMinFilter MinFilter = ImageMinFilter::NEAREST;
		ImageMagFilter MagFilter = ImageMagFilter::NEAREST;
		ImageWrap Wrap_S = ImageWrap::CLAMP_TO_BORDER;
		ImageWrap Wrap_T = ImageWrap::CLAMP_TO_BORDER;
		ImageWrap Wrap_R = ImageWrap::CLAMP_TO_BORDER;
		glm::vec4 Border = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	};

	struct FramebufferAttachmentSpecification
	{
		FramebufferAttachmentSpecification() = default;
		FramebufferAttachmentSpecification(std::initializer_list<FramebufferTextureSpecification> attachments) : TextureAttachments(attachments) {}
		std::vector<FramebufferTextureSpecification> TextureAttachments;
	};

	struct FramebufferSpecification
	{
		uint32_t Width = 0, Height = 0;
		FramebufferAttachmentSpecification Attachments;

		uint32_t Samples = 1;
	};

	class Framebuffer
	{
	public:
		virtual ~Framebuffer() {}

		virtual const FramebufferSpecification& GetSpecification() const = 0;

		virtual uint32_t GetColourAttachmentID(uint32_t index = 0) const = 0;
		virtual void ClearColourAttachment(uint32_t index, const int value) = 0;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void Resize(const uint32_t width, const uint32_t height) = 0;
		virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) = 0;

		static Ref<Framebuffer> Create(const FramebufferSpecification& spec);

		virtual bool operator ==(const Framebuffer* other) const = 0;
		virtual bool operator !=(const Framebuffer* other) const = 0;
	};
}