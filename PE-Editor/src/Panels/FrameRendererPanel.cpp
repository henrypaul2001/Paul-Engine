#include "FrameRendererPanel.h"
#include "PaulEngine/Debug/Instrumentor.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>

namespace PaulEngine
{
	FrameRendererPanel::FrameRendererPanel(const Ref<FrameRenderer>& context)
	{
		SetContext(context);
	}

	void FrameRendererPanel::SetContext(const Ref<FrameRenderer>& context)
	{
		m_Context = context;
	}

	void FrameRendererPanel::OnImGuiRender()
	{
		PE_PROFILE_FUNCTION();

		const std::vector<const char*>& serializedFields = m_Context->GetSerializedComponentNames();

		ImGui::SetNextWindowSizeConstraints(ImVec2(380, 0), ImVec2(FLT_MAX, FLT_MAX), 0, nullptr);
		ImGui::Begin("Frame Renderer");

		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;
		for (const char* name : serializedFields)
		{
			IRenderComponent* iComponent = m_Context->GetRenderResource(name);
			if (iComponent)
			{
				ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
				float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
				ImGui::Separator();
				
				bool open = ImGui::TreeNodeEx(name, treeNodeFlags);
				ImGui::PopStyleVar();

				if (open) {
					const RenderComponentType type = iComponent->GetType();
					iComponent->OnImGuiRender();
					ImGui::TreePop();
				}
			}
		}

		ImGui::End();
	}
}