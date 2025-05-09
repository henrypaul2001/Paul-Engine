#include "pepch.h"
#include "FrameRenderer.h"

namespace PaulEngine
{
	void FrameRenderer::RenderFrame(Ref<Scene> sceneContext, Ref<Camera> activeCamera)
	{
		Ref<Framebuffer> currentTarget = nullptr;
		for (RenderPass& p : m_OrderedRenderPasses) {

			// First check if next render pass uses the same framebuffer as previous pass to avoid state changes
			if (currentTarget.get() && currentTarget.get() != p.m_Context.TargetFramebuffer.get()) {
				if (p.m_Context.TargetFramebuffer) {
					p.m_Context.TargetFramebuffer->Bind();
				}
				else if (currentTarget) {
					currentTarget->Unbind();
				}
			}
			else if (p.m_Context.TargetFramebuffer.get()) {
				p.m_Context.TargetFramebuffer->Bind();
			}
			currentTarget = p.m_Context.TargetFramebuffer;

			p.OnRender(sceneContext, activeCamera);
		}
	}

	void FrameRendererNew::RenderFrame(Ref<Scene> sceneContext, Ref<Camera> activeCamera)
	{
		Ref<Framebuffer> currentTarget = nullptr;
		for (RenderPassNew& p : m_OrderedRenderPasses) {

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

			p.OnRender({ sceneContext, activeCamera }, targetFramebuffer, renderPassInputs);
		}
	}

	bool FrameRendererNew::AddRenderPass(RenderPassNew renderPass, Ref<Framebuffer> targetFramebuffer, std::vector<const char*> inputBindings)
	{
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
}