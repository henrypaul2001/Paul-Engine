#include "pepch.h"
#include "FrameRenderer.h"

#include "PaulEngine/Renderer/FrameRendererSerializer.h"

namespace PaulEngine
{
	void FrameRenderer::RenderFrame(Ref<Scene> sceneContext, Ref<Camera> activeCamera, glm::mat4 cameraWorldTransform)
	{
		PE_PROFILE_FUNCTION();

		m_Profile = FrameRendererProfile();
		uint16_t numFBOChanges = 0;

		Ref<Framebuffer> currentTarget = nullptr;
		for (RenderPass& p : m_OrderedRenderPasses) {
			const UUID& renderID = p.GetRenderID();
			RenderPassParameters params = m_ParameterMap[renderID];

			// First check if next render pass uses the same framebuffer as previous pass to avoid state changes
			const Ref<Framebuffer>& targetFramebuffer = params.TargetFramebuffer;
			if (currentTarget.get() && currentTarget.get() != targetFramebuffer.get()) {
				if (targetFramebuffer) {
					targetFramebuffer->Bind();
					numFBOChanges++;
				}
				else if (currentTarget) {
					currentTarget->Unbind();
					numFBOChanges++;
				}
			}
			else if (targetFramebuffer.get()) {
				targetFramebuffer->Bind();
				numFBOChanges++;
			}
			currentTarget = targetFramebuffer;

			p.OnRender({ sceneContext, activeCamera, cameraWorldTransform }, targetFramebuffer, params.InputComponents);
			m_Profile.OrderedRenderPassProfiles.push_back(p.GetProfile());
		}

		m_Profile.NumFramebufferChanges = numFBOChanges;
	}

	bool FrameRenderer::AddRenderPass(RenderPass renderPass, Ref<Framebuffer> targetFramebuffer, std::vector<std::string> inputBindings)
	{
		PE_PROFILE_FUNCTION();
		const UUID& renderID = renderPass.GetRenderID();
		auto it = m_ParameterMap.find(renderID);
		if (it != m_ParameterMap.end())
		{
			PE_CORE_ERROR("RenderPass with ID '{0}' already exists in FrameRenderer", std::to_string(renderID));
			return false;
		}

		std::vector<IRenderComponent*> inputs;
		inputs.reserve(inputBindings.size());
		for (int i = 0; i < inputBindings.size(); i++)
		{
			const std::string& inputName = inputBindings[i];
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
		m_ParameterMap[renderID] = { targetFramebuffer, inputs };

		m_OrderedRenderPasses.push_back(renderPass);
		return true;
	}

	void FrameRenderer::SaveResourceConfig(const std::filesystem::path& path)
	{
		FrameRendererSerializer::SerializeRenderer(*this, path);
	}

	void FrameRenderer::LoadResourceConfig(const std::filesystem::path& path)
	{
		FrameRendererSerializer::DeserializeRenderer(*this, path);
	}
}