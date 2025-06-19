#include "pepch.h"
#include "FrameRenderer.h"

#include "PaulEngine/Utils/YamlConversions.h"
#include <yaml-cpp/yaml.h>
#include <typeindex>

namespace PaulEngine
{
	namespace RenderComponentSerializer
	{
		template <typename T>
		static inline void SerializePrimitiveComponent(const IRenderComponentPrimitiveType* basePrimitive, YAML::Emitter& out)
		{
			auto* casted = static_cast<const RenderComponentPrimitiveType<T>*>(basePrimitive);
			out << YAML::Key << "Data" << YAML::Value << casted->Data;
		}

		// Primitive type dispatch table
		using DispatchFunc = std::function<void(const IRenderComponentPrimitiveType*, YAML::Emitter& out)>;
		static const std::unordered_map<std::type_index, DispatchFunc> primitiveTypeDispatch = {
			{ typeid(float),		[](const IRenderComponentPrimitiveType* c, YAML::Emitter& o) { SerializePrimitiveComponent<float>(c, o); }},
			{ typeid(glm::vec2),	[](const IRenderComponentPrimitiveType* c, YAML::Emitter& o) { SerializePrimitiveComponent<glm::vec2>(c, o); }},
			{ typeid(glm::vec3),	[](const IRenderComponentPrimitiveType* c, YAML::Emitter& o) { SerializePrimitiveComponent<glm::vec3>(c, o); }},
			{ typeid(glm::vec4),	[](const IRenderComponentPrimitiveType* c, YAML::Emitter& o) { SerializePrimitiveComponent<glm::vec4>(c, o); }},

			{ typeid(int),			[](const IRenderComponentPrimitiveType* c, YAML::Emitter& o) { SerializePrimitiveComponent<int>(c, o); }},
			{ typeid(glm::ivec2),	[](const IRenderComponentPrimitiveType* c, YAML::Emitter& o) { SerializePrimitiveComponent<glm::ivec2>(c, o); }},
			{ typeid(glm::ivec3),	[](const IRenderComponentPrimitiveType* c, YAML::Emitter& o) { SerializePrimitiveComponent<glm::ivec3>(c, o); }},
			{ typeid(glm::ivec4),	[](const IRenderComponentPrimitiveType* c, YAML::Emitter& o) { SerializePrimitiveComponent<glm::ivec4>(c, o); }},

			{ typeid(unsigned int), [](const IRenderComponentPrimitiveType* c, YAML::Emitter& o) { SerializePrimitiveComponent<unsigned int>(c, o); }},
			{ typeid(glm::uvec2),	[](const IRenderComponentPrimitiveType* c, YAML::Emitter& o) { SerializePrimitiveComponent<glm::uvec2>(c, o); }},
			{ typeid(glm::uvec3),	[](const IRenderComponentPrimitiveType* c, YAML::Emitter& o) { SerializePrimitiveComponent<glm::uvec3>(c, o); }},
			{ typeid(glm::uvec4),	[](const IRenderComponentPrimitiveType* c, YAML::Emitter& o) { SerializePrimitiveComponent<glm::uvec4>(c, o); }},

			{ typeid(bool),			[](const IRenderComponentPrimitiveType* c, YAML::Emitter& o) { SerializePrimitiveComponent<bool>(c, o); }}
		};

		static void SerializeComponent(const RenderComponentFramebuffer* framebufferComponent, YAML::Emitter& out)
		{
			PE_CORE_WARN("Framebuffer render component serialization not yet implemented");
		}
		static void SerializeComponent(const RenderComponentTexture* textureComponent, YAML::Emitter& out)
		{
			out << YAML::Key << "TextureAssetHandle" << YAML::Value << (UUID)textureComponent->TextureHandle;
		}
		static void SerializeComponent(const RenderComponentCamera* cameraComponent, YAML::Emitter& out)
		{
			PE_CORE_WARN("Camera render component serialization not yet implemented");
		}
		static void SerializeComponent(const RenderComponentMaterial* materialComponent, YAML::Emitter& out)
		{
			out << YAML::Key << "MaterialAssetHandle" << YAML::Value << (UUID)materialComponent->MaterialHandle;
		}
		static void SerializeComponent(const RenderComponentUBO* uboComponent, YAML::Emitter& out)
		{
			PE_CORE_WARN("UBO render component serialization not yet implemented");
		}
		static void SerializeComponent(const RenderComponentFBOAttachment* fboAttachmentComponent, YAML::Emitter& out)
		{
			PE_CORE_WARN("FBOAttachment render component serialization not yet implemented");
		}
		static void SerializeComponent(const RenderComponentEnvironmentMap* envMapComponent, YAML::Emitter& out)
		{
			out << YAML::Key << "EnvMapAssetHandle" << YAML::Value << (UUID)envMapComponent->EnvironmentHandle;
		}

		static void SerializeComponent(const IRenderComponentPrimitiveType* basePrimitive, YAML::Emitter& out)
		{
			std::type_index typeIndex = basePrimitive->GetPrimitiveTypeInfo();

			auto it = primitiveTypeDispatch.find(typeIndex);
			if (it != primitiveTypeDispatch.end())
			{
				it->second(basePrimitive, out);
			}
			else
			{
				PE_CORE_WARN("Unsupported RenderComponentPrimitiveType serialization");
			}
		}
	}

	void FrameRenderer::RenderFrame(Ref<Scene> sceneContext, Ref<Camera> activeCamera, glm::mat4 cameraWorldTransform)
	{
		PE_PROFILE_FUNCTION();
		Ref<Framebuffer> currentTarget = nullptr;
		for (RenderPass& p : m_OrderedRenderPasses) {

			// First check if next render pass uses the same framebuffer as previous pass to avoid state changes
			const UUID& renderID = p.GetRenderID();
			Ref<Framebuffer> targetFramebuffer = m_FramebufferMap[renderID];
			if (currentTarget.get() && currentTarget.get() != targetFramebuffer.get()) {
				if (targetFramebuffer) {
					targetFramebuffer->Bind();
				}
				else if (currentTarget) {
					currentTarget->Unbind();
				}
			}
			else if (targetFramebuffer.get()) {
				targetFramebuffer->Bind();
			}
			currentTarget = targetFramebuffer;

			std::vector<IRenderComponent*> renderPassInputs = m_InputMap.at(renderID);

			p.OnRender({ sceneContext, activeCamera, cameraWorldTransform }, targetFramebuffer, renderPassInputs);
		}
	}

	bool FrameRenderer::AddRenderPass(RenderPass renderPass, Ref<Framebuffer> targetFramebuffer, std::vector<const char*> inputBindings)
	{
		PE_PROFILE_FUNCTION();
		const UUID& renderID = renderPass.GetRenderID();
		auto it = m_FramebufferMap.find(renderID);
		if (it != m_FramebufferMap.end())
		{
			PE_CORE_ERROR("RenderPass with ID '{0}' already exists in FrameRenderer", std::to_string(renderID));
			return false;
		}

		std::vector<IRenderComponent*> inputs;
		inputs.reserve(inputBindings.size());
		for (int i = 0; i < inputBindings.size(); i++)
		{
			const char* inputName = inputBindings[i];
			if (m_RenderResources.find(inputName) == m_RenderResources.end())
			{
				PE_CORE_ERROR("Unknown render resource '{0}'", inputName);
				return false;
			}
			IRenderComponent* component = m_RenderResources[inputName].get();
			if (component->GetType() == renderPass.GetInputTypes()[i])
			{
				inputs.push_back(m_RenderResources[inputName].get());
			}
			else
			{
				PE_CORE_ERROR("Mismatching input types for render pass with ID '{0}'. Expected: '{1}' ... Actual: '{2}'", std::to_string(renderID),
					RenderComponentTypeString(renderPass.GetInputTypes()[i]), RenderComponentTypeString(m_RenderResources[inputName]->GetType()));
				return false;
			}
		}
		m_FramebufferMap[renderID] = targetFramebuffer;
		m_InputMap[renderID] = inputs;

		m_OrderedRenderPasses.push_back(renderPass);
		return true;
	}

	static void SerializeRenderComponent(const IRenderComponent* component, const char* name, YAML::Emitter& out)
	{
		const RenderComponentType type = component->GetType();
		out << YAML::BeginMap;
		out << YAML::Key << "Name" << YAML::Value << name;
		out << YAML::Key << "Type" << YAML::Value << RenderComponentTypeString(type);
		
		switch (type)
		{
		case RenderComponentType::None:
			out << YAML::Value << "Error type";
			break;
		case RenderComponentType::Framebuffer:
			RenderComponentSerializer::SerializeComponent(static_cast<const RenderComponentFramebuffer*>(component), out);
			break;
		case RenderComponentType::Texture:
			RenderComponentSerializer::SerializeComponent(static_cast<const RenderComponentTexture*>(component), out);
			break;
		case RenderComponentType::Camera:
			RenderComponentSerializer::SerializeComponent(static_cast<const RenderComponentCamera*>(component), out);
			break;
		case RenderComponentType::Material:
			RenderComponentSerializer::SerializeComponent(static_cast<const RenderComponentMaterial*>(component), out);
			break;
		case RenderComponentType::UBO:
			RenderComponentSerializer::SerializeComponent(static_cast<const RenderComponentUBO*>(component), out);
			break;
		case RenderComponentType::FramebufferAttachment:
			RenderComponentSerializer::SerializeComponent(static_cast<const RenderComponentFBOAttachment*>(component), out);
			break;
		case RenderComponentType::PrimitiveType:
			RenderComponentSerializer::SerializeComponent(static_cast<const IRenderComponentPrimitiveType*>(component), out);
			break;
		case RenderComponentType::EnvironmentMap:
			RenderComponentSerializer::SerializeComponent(static_cast<const RenderComponentEnvironmentMap*>(component), out);
			break;
		}

		out << YAML::EndMap;
	}

	void FrameRenderer::SaveResourceConfig(const std::filesystem::path& path)
	{
		PE_PROFILE_FUNCTION();
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "FrameRendererConfig" << YAML::Value << path.stem().c_str();
		out << YAML::Key << "SerializedResources" << YAML::Value << YAML::BeginSeq;

		for (const char* resourceName : m_SerializedComponentNames)
		{
			const IRenderComponent* component = m_RenderResources.at(resourceName).get();
			if (component)
			{
				SerializeRenderComponent(component, resourceName, out);
			}
		}

		out << YAML::EndSeq;
		out << YAML::EndMap;
		
		std::error_code error;
		std::filesystem::create_directories(path.parent_path(), error);
		std::ofstream fout = std::ofstream(path);
		fout << out.c_str();
	}

	void FrameRenderer::ReadResourceConfig(const std::filesystem::path& path)
	{
		PE_PROFILE_FUNCTION();
		//std::ifstream stream = std::ifstream(filepath);
		//std::stringstream ss;
		//ss << stream.rdbuf();
		//
		//YAML::Node data = YAML::Load(ss.str());
		//if (!data["Scene"]) { return false; }
		//
		//std::string sceneName = data["Scene"].as<std::string>();
		//PE_CORE_TRACE("Deserializing scene '{0}'", sceneName);
		//
		//m_Scene->m_Name = sceneName;
		//
		//return true;
	}
}