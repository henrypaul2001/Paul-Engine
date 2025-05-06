#pragma once
#include "PaulEngine/Core/Core.h"
#include "Texture.h"

namespace PaulEngine
{
	enum class FramebufferAttachmentType
	{
		None = 0,
		Texture2D,
		Texture2DArray,
		TextureCubemap,
		TextureCubemapArray,
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

	class Framebuffer;

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

		virtual void BindToFramebuffer(const Framebuffer* targetFramebuffer) = 0;
	};

	class FramebufferTexture2DAttachment : public FramebufferAttachment
	{
	public:
		FramebufferTexture2DAttachment(FramebufferAttachmentPoint attachPoint, Ref<Texture2D> texture) : m_AttachPoint(attachPoint), m_Texture(texture) {}
		FramebufferTexture2DAttachment(FramebufferAttachmentPoint attachPoint, TextureSpecification textureSpec) : m_AttachPoint(attachPoint), m_Texture(Texture2D::Create(textureSpec)) {}
		virtual ~FramebufferTexture2DAttachment() {}

		virtual FramebufferAttachmentPoint GetAttachPoint() const override { return m_AttachPoint; }
		virtual FramebufferAttachmentType GetType() const override { return FramebufferAttachmentType::Texture2D; }

		const Ref<Texture2D> GetTexture() { return m_Texture; }

		virtual void Resize(const uint32_t width, const uint32_t height) override;

		static Ref<FramebufferTexture2DAttachment> Create(FramebufferAttachmentPoint attachPoint, Ref<Texture2D> textureArray);
		static Ref<FramebufferTexture2DAttachment> Create(FramebufferAttachmentPoint attachPoint, TextureSpecification textureSpec);

	protected:
		FramebufferAttachmentPoint m_AttachPoint;
		Ref<Texture2D> m_Texture;
	};

	class FramebufferTexture2DArrayAttachment : public FramebufferAttachment
	{
	public:
		FramebufferTexture2DArrayAttachment(FramebufferAttachmentPoint attachPoint, Ref<Texture2DArray> textureArray) : m_AttachPoint(attachPoint), m_TextureArray(textureArray), m_TargetIndex(0) {}
		FramebufferTexture2DArrayAttachment(FramebufferAttachmentPoint attachPoint, TextureSpecification textureSpec, std::vector<Buffer> layers) : m_AttachPoint(attachPoint), m_TextureArray(Texture2DArray::Create(textureSpec, layers)), m_TargetIndex(0) {}
		virtual ~FramebufferTexture2DArrayAttachment() {}

		virtual FramebufferAttachmentPoint GetAttachPoint() const override { return m_AttachPoint; }
		virtual FramebufferAttachmentType GetType() const override { return FramebufferAttachmentType::Texture2DArray; }

		const Ref<Texture2DArray> GetTexture() { return m_TextureArray; }

		virtual void Resize(const uint32_t width, const uint32_t height) override;

		void SetTargetIndex(uint8_t newTarget) {
			m_TargetIndex = std::min(newTarget, uint8_t(m_TextureArray->GetNumLayers() - 1));
		}

		static Ref<FramebufferTexture2DArrayAttachment> Create(FramebufferAttachmentPoint attachPoint, Ref<Texture2DArray> texture);
		static Ref<FramebufferTexture2DArrayAttachment> Create(FramebufferAttachmentPoint attachPoint, TextureSpecification textureSpec, std::vector<Buffer> layers);

	protected:
		FramebufferAttachmentPoint m_AttachPoint;
		Ref<Texture2DArray> m_TextureArray;
		uint8_t m_TargetIndex;
	};

	class FramebufferTextureCubemapAttachment : public FramebufferAttachment
	{
	public:
		FramebufferTextureCubemapAttachment(FramebufferAttachmentPoint attachPoint, Ref<TextureCubemap> cubemap) : m_AttachPoint(attachPoint), m_Cubemap(cubemap), m_TargetFace(CubemapFace::POSITIVE_X) {}
		FramebufferTextureCubemapAttachment(FramebufferAttachmentPoint attachPoint, TextureSpecification textureSpec, std::vector<Buffer> faceData) : m_AttachPoint(attachPoint), m_Cubemap(TextureCubemap::Create(textureSpec, faceData)), m_TargetFace(CubemapFace::POSITIVE_X) {}
		virtual ~FramebufferTextureCubemapAttachment() {}

		virtual FramebufferAttachmentPoint GetAttachPoint() const override { return m_AttachPoint; }
		virtual FramebufferAttachmentType GetType() const override { return FramebufferAttachmentType::TextureCubemap; }

		const Ref<TextureCubemap> GetCubemap() { return m_Cubemap; }

		virtual void Resize(const uint32_t width, const uint32_t height) override;

		void SetTargetFace(CubemapFace targetFace) { m_TargetFace = targetFace; }

		static Ref<FramebufferTextureCubemapAttachment> Create(FramebufferAttachmentPoint attachPoint, Ref<TextureCubemap> cubemap);
		static Ref<FramebufferTextureCubemapAttachment> Create(FramebufferAttachmentPoint attachPoint, TextureSpecification textureSpec, std::vector<Buffer> faceData);

	protected:
		FramebufferAttachmentPoint m_AttachPoint;
		Ref<TextureCubemap> m_Cubemap;
		CubemapFace m_TargetFace;
	};

	class FramebufferTextureCubemapArrayAttachment : public FramebufferAttachment
	{
	public:
		FramebufferTextureCubemapArrayAttachment(FramebufferAttachmentPoint attachPoint, Ref<TextureCubemapArray> cubemap) : m_AttachPoint(attachPoint), m_CubemapArray(cubemap), m_TargetFace(CubemapFace::POSITIVE_X), m_TargetIndex(0) {}
		FramebufferTextureCubemapArrayAttachment(FramebufferAttachmentPoint attachPoint, TextureSpecification textureSpec, std::vector<std::vector<Buffer>> faceDataLayers) : m_AttachPoint(attachPoint), m_CubemapArray(TextureCubemapArray::Create(textureSpec, faceDataLayers)), m_TargetFace(CubemapFace::POSITIVE_X), m_TargetIndex(0) {}
		virtual ~FramebufferTextureCubemapArrayAttachment() {}

		virtual FramebufferAttachmentPoint GetAttachPoint() const override { return m_AttachPoint; }
		virtual FramebufferAttachmentType GetType() const override { return FramebufferAttachmentType::TextureCubemapArray; }

		const Ref<TextureCubemapArray> GetCubemapArray() { return m_CubemapArray; }

		virtual void Resize(const uint32_t width, const uint32_t height) override;

		bool BindAsLayered = true;
		void SetTargetFace(CubemapFace targetFace) { m_TargetFace = targetFace; }
		void SetTargetIndex(uint8_t newTarget) { m_TargetIndex = std::min(newTarget, uint8_t(m_CubemapArray->GetNumLayers() - 1)); }

		static Ref<FramebufferTextureCubemapArrayAttachment> Create(FramebufferAttachmentPoint attachPoint, Ref<TextureCubemapArray> cubemapArray);
		static Ref<FramebufferTextureCubemapArrayAttachment> Create(FramebufferAttachmentPoint attachPoint, TextureSpecification textureSpec, std::vector<std::vector<Buffer>> faceDataLayers);

	protected:
		FramebufferAttachmentPoint m_AttachPoint;
		Ref<TextureCubemapArray> m_CubemapArray;
		CubemapFace m_TargetFace;
		uint8_t m_TargetIndex;
	};

	class FramebufferRenderbufferAttachment : public FramebufferAttachment
	{
	public:
		FramebufferRenderbufferAttachment(FramebufferAttachmentPoint attachPoint) : m_AttachPoint(attachPoint) {
			PE_CORE_ASSERT(false, "Not yet implemented");
		}
		virtual ~FramebufferRenderbufferAttachment() {}

		virtual FramebufferAttachmentPoint GetAttachPoint() const override { return m_AttachPoint; }
		virtual FramebufferAttachmentType GetType() const override { return FramebufferAttachmentType::Renderbuffer; }

		virtual void Resize(const uint32_t width, const uint32_t height) override { PE_CORE_ASSERT(false, "Not yet implemented"); }

		static Ref<FramebufferRenderbufferAttachment> Create(FramebufferAttachmentPoint attachPoint);

	protected:
		FramebufferAttachmentPoint m_AttachPoint;
	};

	struct FramebufferSpecification
	{
		uint32_t Width = 0, Height = 0;
		uint32_t Samples = 1;
	};

	class Framebuffer
	{
	public:
		virtual ~Framebuffer() {}

		virtual const FramebufferSpecification& GetSpecification() const = 0;

		virtual Ref<FramebufferAttachment> GetAttachment(FramebufferAttachmentPoint attachPoint) = 0;
		virtual Ref<FramebufferAttachment> GetDepthAttachment() = 0;

		virtual bool AddColourAttachment(Ref<FramebufferAttachment> colourAttachment) = 0;
		virtual bool SetDepthAttachment(Ref<FramebufferAttachment> depthAttachment) = 0;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void Resize(const uint32_t width, const uint32_t height) = 0;
		virtual int ReadPixel(FramebufferAttachmentPoint attachPoint, int x, int y) = 0; // If there are no colour buffers and you want to read the depth buffer, glReadBuffer() must be GL_NONE with the data type set to GL_DEPTH_COMPONENT

		static Ref<Framebuffer> Create(const FramebufferSpecification& spec, std::vector<Ref<FramebufferAttachment>> colourAttachments = {}, Ref<FramebufferAttachment> depthAttachment = nullptr);

		virtual bool operator ==(const Framebuffer* other) const = 0;
		virtual bool operator !=(const Framebuffer* other) const = 0;
	};
}
