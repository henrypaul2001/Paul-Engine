#include "PaulEngine/Renderer/Camera.h"
#include "PaulEngine/Renderer/Framebuffer.h"
#include "PaulEngine/Renderer/Texture.h"
#include "PaulEngine/Renderer/Material.h"
#include "PaulEngine/Renderer/UniformBuffer.h"

namespace PaulEngine
{
	enum class RenderComponentType
	{
		None = 0,
		Framebuffer,
		Texture,
		Camera,
		Material,
		UBO,
		PrimitiveType
	};

	static std::string RenderComponentTypeString(RenderComponentType type)
	{
		switch (type)
		{
			case RenderComponentType::None: return "None";
			case RenderComponentType::Framebuffer: return "Framebuffer";
			case RenderComponentType::Texture: return "Texture";
			case RenderComponentType::Camera: return "Camera";
			case RenderComponentType::Material: return "Material";
			case RenderComponentType::UBO: return "UBO";
			case RenderComponentType::PrimitiveType: return "PrimitiveType";
		}
		PE_CORE_ERROR("Unddefined RenderComponentType translation");
		return "Undefined";
	}

	struct IRenderComponent
	{
		virtual ~IRenderComponent() {}
		virtual RenderComponentType GetType() const = 0;
	};

	struct RenderComponentFramebuffer : public IRenderComponent
	{
		RenderComponentFramebuffer(Ref<Framebuffer> framebuffer) : Framebuffer(framebuffer) {}
		RenderComponentFramebuffer(const FramebufferSpecification& spec, std::vector<Ref<FramebufferAttachment>> colourAttachments = {}, Ref<FramebufferAttachment> depthAttachment = nullptr) : Framebuffer(Framebuffer::Create(spec, colourAttachments, depthAttachment)) {}

		virtual RenderComponentType GetType() const override { return RenderComponentType::Framebuffer; }
		Ref<Framebuffer> Framebuffer;
	};

	struct RenderComponentTexture : public IRenderComponent
	{
		RenderComponentTexture(Ref<Texture> texture) : Texture(texture) {}

		virtual RenderComponentType GetType() const override { return RenderComponentType::Texture; }
		Ref<Texture> Texture;
	};

	struct RenderComponentCamera : public IRenderComponent
	{
		RenderComponentCamera(Ref<Camera> camera) : Camera(camera) {}

		virtual RenderComponentType GetType() const override { return RenderComponentType::Camera; }
		Ref<Camera> Camera;
	};

	struct RenderComponentMaterial : public IRenderComponent
	{
		RenderComponentMaterial(Ref<Material> material) : Material(material) {}

		virtual RenderComponentType GetType() const override { return RenderComponentType::Material; }
		Ref<Material> Material;
	};

	struct RenderComponentUBO : public IRenderComponent
	{
		RenderComponentUBO(Ref<UniformBuffer> ubo) : UBO(ubo) {}

		virtual RenderComponentType GetType() const override { return RenderComponentType::UBO; }
		Ref<UniformBuffer> UBO;
	};

	template<typename T>
	struct RenderComponentPrimitiveType : public IRenderComponent
	{
		RenderComponentPrimitiveType(T data) : Data(data) {}

		virtual RenderComponentType GetType() const override { return RenderComponentType::PrimitiveType; }
		T Data;
	};
}