#pragma once
#include "PaulEngine/Renderer/FrameRenderer.h"

#include "PaulEngine/Utils/YamlConversions.h"
#include <yaml-cpp/yaml.h>

#include <typeindex>

namespace PaulEngine
{
	using DispatchFunc = std::function<void(const IRenderComponentPrimitiveType*, YAML::Emitter& out)>;
	class FrameRendererSerializer
	{
	public:
		static void SerializeRenderer(const FrameRenderer& renderer, const std::filesystem::path& path);

	private:
		template <typename T>
		static inline void SerializePrimitiveComponent(const IRenderComponentPrimitiveType* basePrimitive, YAML::Emitter& out)
		{
			auto* casted = static_cast<const RenderComponentPrimitiveType<T>*>(basePrimitive);
			out << YAML::Key << "Data" << YAML::Value << casted->Data;
		}

		// Primitive type dispatch table
		static const std::unordered_map<std::type_index, DispatchFunc> s_PrimitiveTypeDispatch;

		static void SerializeComponent(const RenderComponentFramebuffer* framebufferComponent, YAML::Emitter& out);
		static void SerializeComponent(const RenderComponentTexture* textureComponent, YAML::Emitter& out);
		static void SerializeComponent(const RenderComponentCamera* cameraComponent, YAML::Emitter& out);
		static void SerializeComponent(const RenderComponentMaterial* materialComponent, YAML::Emitter& out);
		static void SerializeComponent(const RenderComponentUBO* uboComponent, YAML::Emitter& out);
		static void SerializeComponent(const RenderComponentFBOAttachment* fboAttachmentComponent, YAML::Emitter& out);
		static void SerializeComponent(const RenderComponentEnvironmentMap* envMapComponent, YAML::Emitter& out);
		static void SerializeComponent(const IRenderComponentPrimitiveType* basePrimitive, YAML::Emitter& out);

		static void SerializeRenderComponent(const IRenderComponent* component, const char* name, YAML::Emitter& out);
	};
}