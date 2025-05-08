#include "PaulEngine/Renderer/Camera.h"
#include "PaulEngine/Renderer/Framebuffer.h"
#include "PaulEngine/Renderer/Texture.h"

namespace PaulEngine
{
	enum class RenderComponentType
	{
		None = 0,
		Framebuffer,
		Texture,
		Camera,
		PrimitiveType
	};

	struct IRenderComponent
	{
		virtual RenderComponentType GetType() const = 0;
	};

	struct RenderComponentFramebuffer : public IRenderComponent
	{
		virtual RenderComponentType GetType() const override { return RenderComponentType::Framebuffer; }
		Ref<Framebuffer> Framebuffer;
	};

	struct RenderComponentTexture : public IRenderComponent
	{
		virtual RenderComponentType GetType() const override { return RenderComponentType::Texture; }
		Ref<Texture> Texture;
	};

	struct RenderComponentCamera : public IRenderComponent
	{
		virtual RenderComponentType GetType() const override { return RenderComponentType::Camera; }
		Ref<Camera> Texture;
	};

	template<typename T>
	struct RenderComponentPrimitiveType : public IRenderComponent
	{
		virtual RenderComponentType GetType() const override { return RenderComponentType::PrimitiveType; }
		T Data;
	};
}