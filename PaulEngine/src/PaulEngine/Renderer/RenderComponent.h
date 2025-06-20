#pragma once
#include "PaulEngine/Renderer/Resource/Camera.h"
#include "PaulEngine/Renderer/Resource/Framebuffer.h"
#include "PaulEngine/Renderer/Asset/Texture.h"
#include "PaulEngine/Renderer/Asset/Material.h"
#include "PaulEngine/Renderer/Resource/UniformBuffer.h"

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
		FramebufferAttachment,
		PrimitiveType,
		EnvironmentMap
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
			case RenderComponentType::FramebufferAttachment: return "FramebufferAttachment";
			case RenderComponentType::PrimitiveType: return "PrimitiveType";
			case RenderComponentType::EnvironmentMap: return "EnvironmentMap";
		}
		PE_CORE_ERROR("Undefined RenderComponentType translation");
		return "Undefined";
	}

	static RenderComponentType StringToRenderComponentType(const std::string& string)
	{
		if (string == "None") { return RenderComponentType::None; }
		if (string == "Framebuffer") { return RenderComponentType::Framebuffer; }
		if (string == "Texture") { return RenderComponentType::Texture; }
		if (string == "Camera") { return RenderComponentType::Camera; }
		if (string == "Material") { return RenderComponentType::Material; }
		if (string == "UBO") { return RenderComponentType::UBO; }
		if (string == "FramebufferAttachment") { return RenderComponentType::FramebufferAttachment; }
		if (string == "PrimitiveType") { return RenderComponentType::PrimitiveType; }
		if (string == "EnvironmentMap") { return RenderComponentType::EnvironmentMap; }

		PE_CORE_ERROR("Undefined RenderComponentType string translation: '{0}'", string.c_str());
		return RenderComponentType::None;
	}

	struct IRenderComponent
	{
		virtual ~IRenderComponent() {}
		virtual RenderComponentType GetType() const = 0;
		virtual void OnImGuiRender() = 0;
	};

	struct RenderComponentFramebuffer : public IRenderComponent
	{
		RenderComponentFramebuffer(Ref<Framebuffer> framebuffer) : Framebuffer(framebuffer) {}
		RenderComponentFramebuffer(const FramebufferSpecification& spec, std::vector<Ref<FramebufferAttachment>> colourAttachments = {}, Ref<FramebufferAttachment> depthAttachment = nullptr) : Framebuffer(Framebuffer::Create(spec, colourAttachments, depthAttachment)) {}

		virtual RenderComponentType GetType() const override { return RenderComponentType::Framebuffer; }
		virtual void OnImGuiRender() override;

		Ref<Framebuffer> Framebuffer;
	};

	struct RenderComponentTexture : public IRenderComponent
	{
		RenderComponentTexture(AssetHandle textureHandle) : TextureHandle(textureHandle) {}

		virtual RenderComponentType GetType() const override { return RenderComponentType::Texture; }
		virtual void OnImGuiRender() override;

		AssetHandle TextureHandle;
	};

	struct RenderComponentCamera : public IRenderComponent
	{
		RenderComponentCamera(Ref<Camera> camera) : Camera(camera) {}

		virtual RenderComponentType GetType() const override { return RenderComponentType::Camera; }
		virtual void OnImGuiRender() override;

		Ref<Camera> Camera;
	};

	struct RenderComponentMaterial : public IRenderComponent
	{
		RenderComponentMaterial(AssetHandle materialHandle) : MaterialHandle(materialHandle) {}

		virtual RenderComponentType GetType() const override { return RenderComponentType::Material; }
		virtual void OnImGuiRender() override;

		AssetHandle MaterialHandle;
	};

	struct RenderComponentUBO : public IRenderComponent
	{
		RenderComponentUBO(Ref<UniformBuffer> ubo) : UBO(ubo) {}

		virtual RenderComponentType GetType() const override { return RenderComponentType::UBO; }
		virtual void OnImGuiRender() override;

		Ref<UniformBuffer> UBO;
	};

	struct RenderComponentFBOAttachment : public IRenderComponent
	{
		RenderComponentFBOAttachment(Ref<FramebufferAttachment> attachment) : Attachment(attachment) {}

		virtual RenderComponentType GetType() const override { return RenderComponentType::FramebufferAttachment; }
		virtual void OnImGuiRender() override;

		Ref<FramebufferAttachment> Attachment;
	};

	struct IRenderComponentPrimitiveType : public IRenderComponent
	{
		virtual const std::type_info& GetPrimitiveTypeInfo() const = 0;
	};

	template<typename T>
	struct RenderComponentPrimitiveType : public IRenderComponentPrimitiveType
	{
		RenderComponentPrimitiveType(T data) : Data(data) {}

		virtual RenderComponentType GetType() const override { return RenderComponentType::PrimitiveType; }
		virtual void OnImGuiRender() override;

		virtual const std::type_info& GetPrimitiveTypeInfo() const override { return typeid(T); }

		T Data;
	};

	struct RenderComponentEnvironmentMap : public IRenderComponent
	{
		RenderComponentEnvironmentMap(AssetHandle envHandle) : EnvironmentHandle(envHandle) {}

		virtual RenderComponentType GetType() const override { return RenderComponentType::EnvironmentMap; }
		virtual void OnImGuiRender() override;

		AssetHandle EnvironmentHandle;
	};

	namespace RenderComponentImGuiUtils
	{
		void DrawNotYetImplemented();

		void DrawEditFloat (float* f, const float speed = 0.1f);
		void DrawEditFloat2(float* f, const float speed = 0.1f);
		void DrawEditFloat3(float* f, const float speed = 0.1f);
		void DrawEditFloat4(float* f, const float speed = 0.1f);

		void DrawEditInt (int* i, const float speed = 1.0f);
		void DrawEditInt2(int* i, const float speed = 1.0f);
		void DrawEditInt3(int* i, const float speed = 1.0f);
		void DrawEditInt4(int* i, const float speed = 1.0f);

		void DrawEditUInt(unsigned int* i, const float speed = 1.0f);
		void DrawEditUInt2(unsigned int* i, const float speed = 1.0f);
		void DrawEditUInt3(unsigned int* i, const float speed = 1.0f);
		void DrawEditUInt4(unsigned int* i, const float speed = 1.0f);

		void DrawCheckbox(bool* b);
	}

	template<typename T>
	inline void RenderComponentPrimitiveType<T>::OnImGuiRender()
	{
		RenderComponentImGuiUtils::DrawNotYetImplemented();
	}

	//  Float
	// -------
	inline void RenderComponentPrimitiveType<float>::OnImGuiRender()
	{
		RenderComponentImGuiUtils::DrawEditFloat(&Data, 0.1f);
	}
	inline void RenderComponentPrimitiveType<glm::vec2>::OnImGuiRender()
	{
		RenderComponentImGuiUtils::DrawEditFloat2(&Data[0], 0.1f);
	}
	inline void RenderComponentPrimitiveType<glm::vec3>::OnImGuiRender()
	{
		RenderComponentImGuiUtils::DrawEditFloat3(&Data[0], 0.1f);
	}
	inline void RenderComponentPrimitiveType<glm::vec4>::OnImGuiRender()
	{
		RenderComponentImGuiUtils::DrawEditFloat4(&Data[0], 0.1f);
	}

	//  Integer
	// ---------
	inline void RenderComponentPrimitiveType<int>::OnImGuiRender()
	{
		RenderComponentImGuiUtils::DrawEditInt(&Data, 0.1f);
	}
	inline void RenderComponentPrimitiveType<glm::ivec2>::OnImGuiRender()
	{
		RenderComponentImGuiUtils::DrawEditInt2(&Data[0], 0.1f);
	}
	inline void RenderComponentPrimitiveType<glm::ivec3>::OnImGuiRender()
	{
		RenderComponentImGuiUtils::DrawEditInt3(&Data[0], 0.1f);
	}
	inline void RenderComponentPrimitiveType<glm::ivec4>::OnImGuiRender()
	{
		RenderComponentImGuiUtils::DrawEditInt4(&Data[0], 0.1f);
	}
	inline void RenderComponentPrimitiveType<unsigned int>::OnImGuiRender()
	{
		RenderComponentImGuiUtils::DrawEditUInt(&Data, 0.1f);
	}
	inline void RenderComponentPrimitiveType<glm::uvec2>::OnImGuiRender()
	{
		RenderComponentImGuiUtils::DrawEditUInt2(&Data[0], 0.1f);
	}
	inline void RenderComponentPrimitiveType<glm::uvec3>::OnImGuiRender()
	{
		RenderComponentImGuiUtils::DrawEditUInt3(&Data[0], 0.1f);
	}
	inline void RenderComponentPrimitiveType<glm::uvec4>::OnImGuiRender()
	{
		RenderComponentImGuiUtils::DrawEditUInt4(&Data[0], 0.1f);
	}

	//  Bool
	// ------
	inline void RenderComponentPrimitiveType<bool>::OnImGuiRender()
	{
		RenderComponentImGuiUtils::DrawCheckbox(&Data);
	}
}