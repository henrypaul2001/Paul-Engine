#pragma once
#include "PaulEngine/Core/Core.h"
#include "Texture.h"

namespace PaulEngine
{
	enum class FramebufferAttachmentType
	{
		None = 0,
		Texture,
		Renderbuffer
	};

	enum class FramebufferAttachmentPoint
	{
		None = 0,
		Colour0,
		Colour1,
		Colour2,
		Colour3,
		Colour4,
		Colour5,
		Colour6,
		Colour7,
		Depth,
		Stencil,
		DepthStencil
	};

	class FramebufferAttachment
	{
	public:
		virtual ~FramebufferAttachment() {}

		virtual FramebufferAttachmentPoint GetAttachPoint() const = 0;
		virtual FramebufferAttachmentType GetType() const = 0;

		static bool IsDepthAttachment(FramebufferAttachmentPoint attachPoint) {
			switch (attachPoint) {
				case FramebufferAttachmentPoint::Depth: return true;
				case FramebufferAttachmentPoint::Stencil: return true;
				case FramebufferAttachmentPoint::DepthStencil: return true;
			}
			return false;
		}

		virtual void Resize(const uint32_t width, const uint32_t height) = 0;
	};

	class FramebufferTexture2DAttachment : public FramebufferAttachment
	{
	public:
		FramebufferTexture2DAttachment(FramebufferAttachmentPoint attachPoint, Ref<Texture2D> texture) : m_AttachPoint(attachPoint), m_Texture(texture) {}
		FramebufferTexture2DAttachment(FramebufferAttachmentPoint attachPoint, TextureSpecification textureSpec) : m_AttachPoint(attachPoint), m_Texture(Texture2D::Create(textureSpec)) {}

		virtual FramebufferAttachmentPoint GetAttachPoint() const override { return m_AttachPoint; }
		virtual FramebufferAttachmentType GetType() const override { return FramebufferAttachmentType::Texture; }

		const Ref<Texture> GetTexture() { return m_Texture; }

		virtual void Resize(const uint32_t width, const uint32_t height) override;

	private:
		FramebufferAttachmentPoint m_AttachPoint;
		Ref<Texture2D> m_Texture;
	};

	class FramebufferRenderbufferAttachment : public FramebufferAttachment
	{
	public:
		FramebufferRenderbufferAttachment(FramebufferAttachmentPoint attachPoint) : m_AttachPoint(attachPoint) {
			PE_CORE_ASSERT(false, "Not yet implemented");
		}

		virtual FramebufferAttachmentPoint GetAttachPoint() const override { return m_AttachPoint; }
		virtual FramebufferAttachmentType GetType() const override { return FramebufferAttachmentType::Renderbuffer; }

		virtual void Resize(const uint32_t width, const uint32_t height) override { PE_CORE_ASSERT(false, "Not yet implemented"); }

	private:
		FramebufferAttachmentPoint m_AttachPoint;
	};

	struct FramebufferSpecification
	{
		uint32_t Width = 0, Height = 0;
		uint32_t Samples = 1;
	};

	class FramebufferNew
	{
	public:
		virtual ~FramebufferNew() {}

		virtual const FramebufferSpecification& GetSpecification() const = 0;

		virtual Ref<FramebufferAttachment> GetAttachment(FramebufferAttachmentPoint attachPoint) = 0;
		virtual Ref<FramebufferAttachment> GetDepthAttachment() = 0;

		virtual bool AddColourAttachment(Ref<FramebufferAttachment> colourAttachment) = 0;
		virtual bool SetDepthAttachment(Ref<FramebufferAttachment> depthAttachment) = 0;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void Resize(const uint32_t width, const uint32_t height) = 0;
		virtual int ReadPixel(FramebufferAttachmentPoint attachPoint, int x, int y) = 0; // If there are no colour buffers and you want to read the depth buffer, glReadBuffer() must be GL_NONE with the data type set to GL_DEPTH_COMPONENT

		static Ref<FramebufferNew> Create(const FramebufferSpecification& spec, std::vector<Ref<FramebufferAttachment>> colourAttachments = {}, Ref<FramebufferAttachment> depthAttachment = nullptr);

		virtual bool operator ==(const FramebufferNew* other) const = 0;
		virtual bool operator !=(const FramebufferNew* other) const = 0;
	};
}
