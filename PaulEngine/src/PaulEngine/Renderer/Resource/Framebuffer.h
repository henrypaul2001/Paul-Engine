#pragma once
#include "PaulEngine/Core/Core.h"
#include "PaulEngine/Asset/AssetManager.h"
#include "../Asset/Texture.h"

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
		FramebufferTexture2DAttachment(FramebufferAttachmentPoint attachPoint, AssetHandle textureHandle) : m_AttachPoint(attachPoint), m_TextureHandle(textureHandle) {
			PE_CORE_ASSERT(AssetManager::IsAssetHandleValid(m_TextureHandle), "Invalid texture handle");
		}
		FramebufferTexture2DAttachment(FramebufferAttachmentPoint attachPoint, TextureSpecification textureSpec, bool persistentAsset = false) : m_AttachPoint(attachPoint), m_TextureHandle(AssetManager::CreateAsset<Texture2D>(persistentAsset, textureSpec)->Handle) {}
		virtual ~FramebufferTexture2DAttachment() {}

		virtual FramebufferAttachmentPoint GetAttachPoint() const override { return m_AttachPoint; }
		virtual FramebufferAttachmentType GetType() const override { return FramebufferAttachmentType::Texture2D; }

		AssetHandle GetTextureHandle() const { return m_TextureHandle; }
		const Ref<Texture2D> GetTexture() const { return AssetManager::GetAsset<Texture2D>(m_TextureHandle); }

		virtual void Resize(const uint32_t width, const uint32_t height) override;

		uint8_t TargetMipLevel = 0;

		static Ref<FramebufferTexture2DAttachment> Create(FramebufferAttachmentPoint attachPoint, AssetHandle textureHandle);
		static Ref<FramebufferTexture2DAttachment> Create(FramebufferAttachmentPoint attachPoint, TextureSpecification textureSpec, bool persistentAsset = false);

	protected:
		FramebufferAttachmentPoint m_AttachPoint;
		AssetHandle m_TextureHandle;
	};

	class FramebufferTexture2DArrayAttachment : public FramebufferAttachment
	{
	public:
		FramebufferTexture2DArrayAttachment(FramebufferAttachmentPoint attachPoint, AssetHandle textureArrayHandle) : m_AttachPoint(attachPoint), m_TextureArrayHandle(textureArrayHandle), m_TargetIndex(0) {
			PE_CORE_ASSERT(AssetManager::IsAssetHandleValid(m_TextureArrayHandle), "Invalid texture handle");
			m_NumLayers = AssetManager::GetAsset<Texture2DArray>(m_TextureArrayHandle)->GetNumLayers();
		}
		FramebufferTexture2DArrayAttachment(FramebufferAttachmentPoint attachPoint, TextureSpecification textureSpec, std::vector<Buffer> layers, bool persistentAsset = false) : m_AttachPoint(attachPoint), m_TextureArrayHandle(AssetManager::CreateAsset<Texture2DArray>(persistentAsset, textureSpec, layers)->Handle), m_TargetIndex(0), m_NumLayers(layers.size()) {}
		virtual ~FramebufferTexture2DArrayAttachment() {}

		virtual FramebufferAttachmentPoint GetAttachPoint() const override { return m_AttachPoint; }
		virtual FramebufferAttachmentType GetType() const override { return FramebufferAttachmentType::Texture2DArray; }

		AssetHandle GetTextureHandle() const { return m_TextureArrayHandle; }
		const Ref<Texture2DArray> GetTexture() const { return AssetManager::GetAsset<Texture2DArray>(m_TextureArrayHandle); }

		virtual void Resize(const uint32_t width, const uint32_t height) override;

		uint8_t TargetMipLevel = 0;
		void SetTargetIndex(uint8_t newTarget) {
			m_TargetIndex = std::min(newTarget, uint8_t(m_NumLayers - 1));
		}

		static Ref<FramebufferTexture2DArrayAttachment> Create(FramebufferAttachmentPoint attachPoint, AssetHandle textureArrayHandle);
		static Ref<FramebufferTexture2DArrayAttachment> Create(FramebufferAttachmentPoint attachPoint, TextureSpecification textureSpec, std::vector<Buffer> layers, bool persistentAsset = false);

	protected:
		FramebufferAttachmentPoint m_AttachPoint;
		AssetHandle m_TextureArrayHandle;
		uint8_t m_TargetIndex;
		uint8_t m_NumLayers;
	};

	class FramebufferTextureCubemapAttachment : public FramebufferAttachment
	{
	public:
		FramebufferTextureCubemapAttachment(FramebufferAttachmentPoint attachPoint, AssetHandle cubemapHandle) : m_AttachPoint(attachPoint), m_CubemapHandle(cubemapHandle), m_TargetFace(CubemapFace::POSITIVE_X) {
			PE_CORE_ASSERT(AssetManager::IsAssetHandleValid(cubemapHandle), "Invalid texture handle");
		}
		FramebufferTextureCubemapAttachment(FramebufferAttachmentPoint attachPoint, TextureSpecification textureSpec, std::vector<Buffer> faceData, bool persistentAsset = false) : m_AttachPoint(attachPoint), m_CubemapHandle(AssetManager::CreateAsset<TextureCubemap>(persistentAsset, textureSpec, faceData)->Handle), m_TargetFace(CubemapFace::POSITIVE_X) {}
		virtual ~FramebufferTextureCubemapAttachment() {}

		virtual FramebufferAttachmentPoint GetAttachPoint() const override { return m_AttachPoint; }
		virtual FramebufferAttachmentType GetType() const override { return FramebufferAttachmentType::TextureCubemap; }

		AssetHandle GetCubemapHandle() const { return m_CubemapHandle; }
		const Ref<TextureCubemap> GetCubemap() const { return AssetManager::GetAsset<TextureCubemap>(m_CubemapHandle); }

		virtual void Resize(const uint32_t width, const uint32_t height) override;

		bool BindAsLayered = true;
		uint8_t TargetMipLevel = 0;
		void SetTargetFace(CubemapFace targetFace) { m_TargetFace = targetFace; }

		static Ref<FramebufferTextureCubemapAttachment> Create(FramebufferAttachmentPoint attachPoint, AssetHandle cubemapHandle);
		static Ref<FramebufferTextureCubemapAttachment> Create(FramebufferAttachmentPoint attachPoint, TextureSpecification textureSpec, std::vector<Buffer> faceData, bool persistentAsset = false);

	protected:
		FramebufferAttachmentPoint m_AttachPoint;
		AssetHandle m_CubemapHandle;
		CubemapFace m_TargetFace;
	};

	class FramebufferTextureCubemapArrayAttachment : public FramebufferAttachment
	{
	public:
		FramebufferTextureCubemapArrayAttachment(FramebufferAttachmentPoint attachPoint, AssetHandle cubemapArrayHandle) : m_AttachPoint(attachPoint), m_CubemapHandle(cubemapArrayHandle), m_TargetFace(CubemapFace::POSITIVE_X), m_TargetIndex(0) {
			PE_CORE_ASSERT(AssetManager::IsAssetHandleValid(m_CubemapHandle), "Invalid texture handle");
			m_NumLayers = AssetManager::GetAsset<TextureCubemapArray>(m_CubemapHandle)->GetNumLayers();
		}
		FramebufferTextureCubemapArrayAttachment(FramebufferAttachmentPoint attachPoint, TextureSpecification textureSpec, std::vector<std::vector<Buffer>> faceDataLayers, bool persistentAsset = false) : m_AttachPoint(attachPoint), m_CubemapHandle(AssetManager::CreateAsset<TextureCubemapArray>(persistentAsset, textureSpec, faceDataLayers)->Handle), m_TargetFace(CubemapFace::POSITIVE_X), m_TargetIndex(0), m_NumLayers(faceDataLayers.size()) {}
		virtual ~FramebufferTextureCubemapArrayAttachment() {}

		virtual FramebufferAttachmentPoint GetAttachPoint() const override { return m_AttachPoint; }
		virtual FramebufferAttachmentType GetType() const override { return FramebufferAttachmentType::TextureCubemapArray; }

		AssetHandle GetCubemapHandle() const { return m_CubemapHandle; }
		const Ref<TextureCubemapArray> GetCubemapArray() { return AssetManager::GetAsset<TextureCubemapArray>(m_CubemapHandle); }

		virtual void Resize(const uint32_t width, const uint32_t height) override;

		bool BindAsLayered = true;
		uint8_t TargetMipLevel = 0;
		void SetTargetFace(CubemapFace targetFace) { m_TargetFace = targetFace; }
		void SetTargetIndex(uint8_t newTarget) { m_TargetIndex = std::min(newTarget, uint8_t(m_NumLayers - 1)); }

		static Ref<FramebufferTextureCubemapArrayAttachment> Create(FramebufferAttachmentPoint attachPoint, AssetHandle cubemapArrayHandle);
		static Ref<FramebufferTextureCubemapArrayAttachment> Create(FramebufferAttachmentPoint attachPoint, TextureSpecification textureSpec, std::vector<std::vector<Buffer>> faceDataLayers, bool persistentAsset = false);

	protected:
		FramebufferAttachmentPoint m_AttachPoint;
		AssetHandle m_CubemapHandle;
		CubemapFace m_TargetFace;
		uint8_t m_TargetIndex;
		uint8_t m_NumLayers;
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
		enum BufferBit
		{
			None = 0,
			COLOUR = BIT(0),
			DEPTH = BIT(1),
			STENCIL = BIT(2)
		};

		enum class BlitFilter
		{
			Nearest,
			Linear
		};

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

		virtual void SetDrawBuffers(std::vector<FramebufferAttachmentPoint> colourBuffers) = 0;
		virtual void SetDrawBuffers() = 0;

		void BlitTo(Framebuffer* targetFramebuffer, int bufferMask = (BufferBit::COLOUR | BufferBit::DEPTH | BufferBit::STENCIL), BlitFilter filtering = BlitFilter::Nearest)
		{
			PE_CORE_ASSERT(targetFramebuffer, "Invalid target framebuffer");
			const FramebufferSpecification& sourceSpec = GetSpecification();
			const FramebufferSpecification& targetSpec = targetFramebuffer->GetSpecification();
			glm::ivec2 sourceMin = { 0, 0 };
			glm::ivec2 sourceMax = { sourceSpec.Width, sourceSpec.Height };
			glm::ivec2 targetMin = { 0, 0 };
			glm::ivec2 targetMax = { targetSpec.Width, targetSpec.Height };

			BlitTo(targetFramebuffer, bufferMask, filtering, sourceMin, sourceMax, targetMin, targetMax);
		}
		virtual void BlitTo(Framebuffer* targetFramebuffer, int bufferMask, BlitFilter filtering, glm::ivec2 sourceRegionMin, glm::ivec2 sourceRegionMax, glm::ivec2 targetRegionMin, glm::ivec2 targetRegionMax) = 0;
		static void Blit(Framebuffer* source, Framebuffer* target, int bufferMask = (BufferBit::COLOUR | BufferBit::DEPTH | BufferBit::STENCIL), BlitFilter filtering = BlitFilter::Nearest) {
			PE_CORE_ASSERT(source, "Invalid source framebuffer");
			source->BlitTo(target, bufferMask, filtering);
		}
		static void Blit(Framebuffer* source, Framebuffer* target, int bufferMask, BlitFilter filtering, glm::ivec2 sourceRegionMin, glm::ivec2 sourceRegionMax, glm::ivec2 targetRegionMin, glm::ivec2 targetRegionMax) {
			PE_CORE_ASSERT(source, "Invalid source framebuffer");
			source->BlitTo(target, bufferMask, filtering, sourceRegionMin, sourceRegionMax, targetRegionMin, targetRegionMax);
		}

		static Ref<Framebuffer> Create(const FramebufferSpecification& spec, std::vector<Ref<FramebufferAttachment>> colourAttachments = {}, Ref<FramebufferAttachment> depthAttachment = nullptr);

		virtual bool operator ==(const Framebuffer* other) const = 0;
		virtual bool operator !=(const Framebuffer* other) const = 0;
	};
}
