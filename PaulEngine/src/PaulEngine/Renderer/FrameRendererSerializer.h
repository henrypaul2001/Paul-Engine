#pragma once
#include "PaulEngine/Renderer/FrameRenderer.h"

#include "PaulEngine/Utils/YamlConversions.h"
#include <yaml-cpp/yaml.h>

#include <typeindex>

namespace PaulEngine
{
	using DispatchSerializeFunc = std::function<void(const IRenderComponentPrimitiveType*, YAML::Emitter& out)>;
	using DispatchDeserializeFunc = std::function<void(IRenderComponentPrimitiveType*, const YAML::Node& componentNode)>;
	class FrameRendererSerializer
	{
	public:
		static void SerializeRenderer(const FrameRenderer& renderer, const std::filesystem::path& path);
		static bool DeserializeRenderer(FrameRenderer& renderer, const std::filesystem::path& path);

	private:
		template <typename T>
		static inline void SerializePrimitiveComponent(const IRenderComponentPrimitiveType* basePrimitive, YAML::Emitter& out)
		{
			auto* casted = static_cast<const RenderComponentPrimitiveType<T>*>(basePrimitive);
			out << YAML::Key << "Data" << YAML::Value << casted->Data;
		}

		template <typename T>
		static inline void DeserializePrimitiveComponent(IRenderComponentPrimitiveType* basePrimitive, const YAML::Node& componentNode)
		{
			auto* casted = static_cast<RenderComponentPrimitiveType<T>*>(basePrimitive);
			const YAML::Node& dataNode = componentNode["Data"];
			if (dataNode)
			{
				casted->Data = dataNode.as<T>();
			}
		}

		// Primitive type dispatch table
		static const std::unordered_map<std::type_index, DispatchSerializeFunc> s_PrimitiveTypeSerializeDispatcher;
		static const std::unordered_map<std::type_index, DispatchDeserializeFunc> s_PrimitiveTypeDeserializeDispatcher;

		// Serialize
		// ---------
		static void SerializeComponent(const RenderComponentFramebuffer* framebufferComponent, YAML::Emitter& out);
		static void SerializeComponent(const RenderComponentTexture* textureComponent, YAML::Emitter& out);
		static void SerializeComponent(const RenderComponentCamera* cameraComponent, YAML::Emitter& out);
		static void SerializeComponent(const RenderComponentMaterial* materialComponent, YAML::Emitter& out);
		static void SerializeComponent(const RenderComponentUBO* uboComponent, YAML::Emitter& out);
		static void SerializeComponent(const RenderComponentFBOAttachment* fboAttachmentComponent, YAML::Emitter& out);
		static void SerializeComponent(const RenderComponentEnvironmentMap* envMapComponent, YAML::Emitter& out);
		static void SerializeComponent(const IRenderComponentPrimitiveType* basePrimitive, YAML::Emitter& out);

		static void SerializeRenderComponent(const IRenderComponent* component, const char* name, YAML::Emitter& out);

		// Deserialize
		// -----------
		static void DeserializeComponent(RenderComponentFramebuffer* framebufferComponent, const YAML::Node& componentNode);
		static void DeserializeComponent(RenderComponentTexture* textureComponent, const YAML::Node& componentNode);
		static void DeserializeComponent(RenderComponentCamera* cameraComponent, const YAML::Node& componentNode);
		static void DeserializeComponent(RenderComponentMaterial* materialComponent, const YAML::Node& componentNode);
		static void DeserializeComponent(RenderComponentUBO* uboComponent, const YAML::Node& componentNode);
		static void DeserializeComponent(RenderComponentFBOAttachment* fboAttachmentComponent, const YAML::Node& componentNode);
		static void DeserializeComponent(RenderComponentEnvironmentMap* envMapComponent, const YAML::Node& componentNode);
		static void DeserializeComponent(IRenderComponentPrimitiveType* basePrimitive, const YAML::Node& componentNode);

		static void DeserializeRenderComponent(IRenderComponent* component, const YAML::Node& componentNode);
	};
}