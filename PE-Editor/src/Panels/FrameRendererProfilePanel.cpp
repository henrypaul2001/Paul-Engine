#include "FrameRendererProfilePanel.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>

#include <PaulEngine/Debug/Instrumentor.h>

namespace PaulEngine
{
	FrameRendererProfilePanel::FrameRendererProfilePanel(Ref<const FrameRenderer> context)
	{
		SetContext(context);
	}

	void FrameRendererProfilePanel::SetContext(Ref<const FrameRenderer> context)
	{
		m_Context = context;
	}

	void FrameRendererProfilePanel::OnImGuiRender()
	{
		PE_PROFILE_FUNCTION();

		ImGui::Begin("Frame Renderer Profile");

		FrameRendererProfile fullProfile = m_Context->GetProfile();
		const std::vector<RenderPass>& orderedRenderPasses = m_Context->GetOrderedRenderPasses();
		const size_t numPasses = fullProfile.NumPasses();
		ImGui::Text("Total render passes: %d", numPasses);
		ImGui::Text("Total framebuffer changes: %d", fullProfile.NumFramebufferChanges);
		ImGui::Text("Total draw calls: %d", fullProfile.TotalDrawCalls);

		ImGui::SeparatorText("Render pass details");
		
		for (int i = 0; i < numPasses; i++)
		{
			const std::string& passName = orderedRenderPasses[i].GetDebugName();
			DrawPassProfile(passName, fullProfile.OrderedRenderPassProfiles[i]);
		}

		ImGui::End();
	}

	void FrameRendererProfilePanel::DrawPassProfile(const std::string& name, const RenderPassProfile& profile)
	{
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;
		ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImGui::Separator();

		bool open = (ImGui::TreeNodeEx(name.c_str(), treeNodeFlags));
		ImGui::PopStyleVar();

		if (open) {
			
			//   2D  
			// ------
			ImGui::Text("2D Stats");
			ImGui::BulletText("Draw calls: %d", profile.DrawCalls2D);
			ImGui::BulletText("Quad count: %d", profile.QuadCount2D);
			ImGui::BulletText("Circle count: %d", profile.CircleCount2D);
			ImGui::BulletText("Line count: %d", profile.LineCount2D);

			//   3D  
			// ------
			ImGui::Text("3D Stats");
			ImGui::BulletText("Draw calls: %d", profile.DrawCalls3D);
			ImGui::BulletText("Mesh count: %d", profile.MeshCount);
			ImGui::BulletText("Unique pipelines: %d", profile.UniquePipelines);

			ImGui::TreePop();
		}
	}
}