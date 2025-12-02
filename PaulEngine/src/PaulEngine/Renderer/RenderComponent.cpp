#include "pepch.h"
#include "RenderComponent.h"

#include <yaml-cpp/yaml.h>
#include <imgui.h>

namespace PaulEngine
{
    void RenderComponentFramebuffer::OnImGuiRender()
    {
        ImGui::Text("Not yet implemented");

        // maybe display the framebuffer spec
        //const FramebufferSpecification& spec = Framebuffer->GetSpecification();
    }

    void RenderComponentTexture::OnImGuiRender()
    {
		std::string label = "None";
		bool isTextureValid = false;
		if (TextureHandle != 0) {
			if (AssetManager::IsAssetHandleValid(TextureHandle) && Asset::IsTextureType(AssetManager::GetAssetType(TextureHandle))) {
				const AssetMetadata& metadata = Project::GetActive()->GetEditorAssetManager()->GetMetadata(TextureHandle);
				label = metadata.FilePath.filename().string();
				isTextureValid = true;
			}
			else {
				label = "Invalid";
			}
		}

		ImVec2 buttonLabelSize = ImGui::CalcTextSize(label.c_str());
		buttonLabelSize.x += 20.0f;
		float buttonLabelWidth = glm::max<float>(100.0f, buttonLabelSize.x);

		ImGui::PushID(0);
		ImGui::Button(label.c_str(), ImVec2(buttonLabelWidth, 0.0f));
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				AssetHandle handle = *(AssetHandle*)payload->Data;
				if (Asset::IsTextureType(AssetManager::GetAssetType(handle))) {
					TextureHandle = handle;
				}
				else {
					PE_CORE_WARN("Invalid asset type. Texture needed for RenderComponentTexture");
				}
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::PopID();

		if (isTextureValid) {
			ImGui::SameLine();
			ImVec2 xLabelSize = ImGui::CalcTextSize("X");
			float buttonSize = xLabelSize.y + ImGui::GetStyle().FramePadding.y * 2.0f;
			if (ImGui::Button("X", ImVec2(buttonSize, buttonSize))) {
				TextureHandle = 0;
			}
		}
		ImGui::SameLine();
		ImGui::Text("Texture");
    }

    void RenderComponentCamera::OnImGuiRender()
    {
		ImGui::Text("Not yet implemented");
    }

    void RenderComponentUBO::OnImGuiRender()
    {
		ImGui::Text("Not yet implemented");
		// there isnt really much to implement here other than UBO initialisation stuff that would require regenerating the UBO
    }

    void RenderComponentFBOAttachment::OnImGuiRender()
    {
		ImGui::Text("Not yet implemented");
		// again, not currently much to add here other than initialisation stuff
    }

	void RenderComponentImGuiUtils::DrawNotYetImplemented()
	{
		ImGui::Text("Not yet implemented");
	}

	void RenderComponentImGuiUtils::DrawEditFloat(float* f, const float speed)
	{
		ImGui::DragFloat("##", f, speed);
	}

	void RenderComponentImGuiUtils::DrawEditFloat2(float* f, const float speed)
	{
		ImGui::DragFloat2("##", f, speed);
	}

	void RenderComponentImGuiUtils::DrawEditFloat3(float* f, const float speed)
	{
		ImGui::DragFloat3("##", f, speed);
	}

	void RenderComponentImGuiUtils::DrawEditFloat4(float* f, const float speed)
	{
		ImGui::DragFloat4("##", f, speed);
	}

	void RenderComponentImGuiUtils::DrawEditInt(int* i, const float speed)
	{
		ImGui::DragInt("##", i, speed);
	}

	void RenderComponentImGuiUtils::DrawEditInt2(int* i, const float speed)
	{
		ImGui::DragInt2("##", i, speed);
	}

	void RenderComponentImGuiUtils::DrawEditInt3(int* i, const float speed)
	{
		ImGui::DragInt3("##", i, speed);
	}

	void RenderComponentImGuiUtils::DrawEditInt4(int* i, const float speed)
	{
		ImGui::DragInt4("##", i, speed);
	}

	void RenderComponentImGuiUtils::DrawEditUInt(unsigned int* i, const float speed)
	{
		ImGui::DragInt("##", (int*)i, speed, 0);
	}

	void RenderComponentImGuiUtils::DrawEditUInt2(unsigned int* i, const float speed)
	{
		ImGui::DragInt2("##", (int*)i, speed, 0);
	}

	void RenderComponentImGuiUtils::DrawEditUInt3(unsigned int* i, const float speed)
	{
		ImGui::DragInt3("##", (int*)i, speed, 0);
	}

	void RenderComponentImGuiUtils::DrawEditUInt4(unsigned int* i, const float speed)
	{
		ImGui::DragInt4("##", (int*)i, speed, 0);
	}

	void RenderComponentImGuiUtils::DrawCheckbox(bool* b)
	{
		ImGui::Checkbox("##", b);
	}

	void RenderComponentImGuiUtils::DrawAssetHandleDragDrop(AssetHandle& targetHandle, AssetType allowedType, const char* title)
	{
		std::string label = "None";
		bool isTextureValid = false;
		if (targetHandle != 0) {
			if (AssetManager::IsAssetHandleValid(targetHandle) && AssetManager::GetAssetType(targetHandle) == allowedType) {
				const AssetMetadata& metadata = Project::GetActive()->GetEditorAssetManager()->GetMetadata(targetHandle);
				label = metadata.FilePath.filename().string();
				isTextureValid = true;
			}
			else {
				label = "Invalid";
			}
		}

		ImVec2 buttonLabelSize = ImGui::CalcTextSize(label.c_str());
		buttonLabelSize.x += 20.0f;
		float buttonLabelWidth = glm::max<float>(100.0f, buttonLabelSize.x);

		ImGui::Button(label.c_str(), ImVec2(buttonLabelWidth, 0.0f));
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				AssetHandle handle = *(AssetHandle*)payload->Data;
				if (AssetManager::GetAssetType(handle) == allowedType) {
					targetHandle = handle;
				}
				else {
					std::string warnString = "Invalid asset type. " + AssetTypeToString(allowedType) + " required";
					PE_CORE_WARN(warnString.c_str());
				}
			}
			ImGui::EndDragDropTarget();
		}

		if (isTextureValid) {
			ImGui::SameLine();
			ImVec2 xLabelSize = ImGui::CalcTextSize("X");
			float buttonSize = xLabelSize.y + ImGui::GetStyle().FramePadding.y * 2.0f;
			if (ImGui::Button("X", ImVec2(buttonSize, buttonSize))) {
				targetHandle = 0;
			}
		}
		ImGui::SameLine();
		ImGui::Text(title);
	}
}