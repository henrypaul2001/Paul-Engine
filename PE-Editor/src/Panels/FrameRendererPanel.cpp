#include "FrameRendererPanel.h"
#include "PaulEngine/Debug/Instrumentor.h"
#include "PaulEngine/Utils/PlatformUtils.h"

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

		const std::vector<std::string>& serializedFields = m_Context->GetSerializedComponentNames();

		ImGui::SetNextWindowSizeConstraints(ImVec2(380, 0), ImVec2(FLT_MAX, FLT_MAX), 0, nullptr);
		ImGui::Begin("Frame Renderer");

		if (ImGui::Button("Save As...")) {
			std::string path = FileDialogs::SaveFile("Render Resource Config (*.rrc)\0*.rrc\0");
			if (!path.empty()) {
				std::filesystem::path absoluteProjectPath = std::filesystem::absolute(Project::GetProjectDirectory());
				std::filesystem::path relativeSavePath = std::filesystem::path(path).lexically_relative(absoluteProjectPath.parent_path());

				m_Context->SaveResourceConfig(relativeSavePath);
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Load...")) {
			std::string path = FileDialogs::OpenFile("Render Resource Config (*.rrc)\0*.rrc\0");
			if (!path.empty()) {
				std::filesystem::path absoluteProjectPath = std::filesystem::absolute(Project::GetProjectDirectory());
				std::filesystem::path relativeSavePath = std::filesystem::path(path).lexically_relative(absoluteProjectPath.parent_path());

				m_Context->LoadResourceConfig(relativeSavePath);
			}
		}

		ImGui::Separator();

		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;
		for (const std::string& name : serializedFields)
		{
			IRenderComponent* iComponent = m_Context->GetRenderResource(name);
			if (iComponent)
			{
				ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
				float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
				ImGui::Separator();
				
				bool open = ImGui::TreeNodeEx(name.c_str(), treeNodeFlags);
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