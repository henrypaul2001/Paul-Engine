#include "pepch.h"
#include "CreateMaterialWindow.h"

#include "PaulEngine/Renderer/Renderer.h"
#include "PaulEngine/Renderer/RenderCommand.h"

#include "PaulEngine/Asset/AssetManager.h"
#include "PaulEngine/Asset/MaterialImporter.h"
#include "PaulEngine/Renderer/Material.h"

#include "PaulEngine/Utils/PlatformUtils.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <glm/glm.hpp>
#include <imgui_internal.h>

namespace PaulEngine
{
	static glm::mat4 s_CubeTransform = glm::mat4(1.0f);
	CreateMaterialWindow::CreateMaterialWindow(AssetHandle shaderHandle) : m_ShaderHandle(shaderHandle), m_ViewportSize(128.0f, 128.0f) {}

	void CreateMaterialWindow::Init()
	{
		FramebufferSpecification spec;
		spec.Width = 1280;
		spec.Height = 720;
		spec.Samples = 1;

		TextureSpecification texSpec;
		texSpec.Width = 1280;
		texSpec.Height = 720;
		texSpec.GenerateMips = false;
		texSpec.Format = ImageFormat::RGBA8;
		texSpec.MinFilter = ImageMinFilter::NEAREST;
		texSpec.MagFilter = ImageMagFilter::NEAREST;
		texSpec.Wrap_S = ImageWrap::CLAMP_TO_BORDER;
		texSpec.Wrap_T = ImageWrap::CLAMP_TO_BORDER;
		texSpec.Wrap_R = ImageWrap::CLAMP_TO_BORDER;
		texSpec.Border = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		Ref<FramebufferTexture2DAttachment> colour0Attach = FramebufferTexture2DAttachment::Create(FramebufferAttachmentPoint::Colour0, texSpec);

		texSpec.Format = ImageFormat::RED_INTEGER;
		Ref<FramebufferTexture2DAttachment> entityIDAttach = FramebufferTexture2DAttachment::Create(FramebufferAttachmentPoint::Colour1, texSpec);

		texSpec.Format = ImageFormat::Depth24Stencil8;
		Ref<FramebufferTexture2DAttachment> depthAttach = FramebufferTexture2DAttachment::Create(FramebufferAttachmentPoint::DepthStencil, texSpec);

		m_Framebuffer = Framebuffer::Create(spec, { colour0Attach, entityIDAttach }, depthAttach);

		m_Camera = SceneCamera(SCENE_CAMERA_PERSPECTIVE);

		s_CubeTransform = glm::mat4(1.0f);
		s_CubeTransform = glm::translate(s_CubeTransform, glm::vec3(0.0f, 0.0f, -1.5f));
		s_CubeTransform = glm::rotate(s_CubeTransform, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	}

	void CreateMaterialWindow::SetContext(AssetHandle shaderHandle)
	{
		m_ShaderHandle = shaderHandle;
	}

	void CreateMaterialWindow::OnImGuiRender()
	{
		PE_PROFILE_FUNCTION();
		if (m_ShowWindow) {
			ImGui::Begin("Create material asset");

			std::string label = "None";
			bool isShaderValid = false;
			if (m_ShaderHandle != 0)
			{
				if (AssetManager::IsAssetHandleValid(m_ShaderHandle) && AssetManager::GetAssetType(m_ShaderHandle) == AssetType::Shader)
				{
					const AssetMetadata& metadata = Project::GetActive()->GetEditorAssetManager()->GetMetadata(m_ShaderHandle);
					label = metadata.FilePath.filename().string();
					isShaderValid = true;
				}
				else { label = "Invalid"; }
			}

			// Engine shader drop down
			Ref<EditorAssetManager> assetManager = Project::GetActive()->GetEditorAssetManager();
			std::filesystem::path engineAssetsRelativeToProjectAssets = std::filesystem::path("assets").lexically_relative(Project::GetAssetDirectory());

			static std::unordered_map<const char*, std::filesystem::path> nameToFilepath;
			nameToFilepath["DefaultLit"] = engineAssetsRelativeToProjectAssets / "shaders/Renderer3D_DefaultLit.glsl";

			static const char* shaderNames[] = {
				"DefaultLit",
			};

			if (m_DropDownShader != -1) {
				label = shaderNames[m_DropDownShader];
			}
			if (ImGui::BeginCombo("##EngineShaderDropDown", label.c_str())) {

				for (int i = 0; i < 1; i++) {
					bool isSelected = m_DropDownShader == i;
					if (ImGui::Selectable(shaderNames[i], isSelected)) {
						m_DropDownShader = i;
						m_ShaderHandle = assetManager->ImportAssetFromFile(nameToFilepath[shaderNames[i]], true);
						m_Material = CreateRef<Material>(m_ShaderHandle);
					}

					if (isSelected) {
						ImGui::SetItemDefaultFocus();
					}
				}

				ImGui::EndCombo();
			}
			
			// Custom shader drag/drop target
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					AssetHandle handle = *(AssetHandle*)payload->Data;
					if (AssetManager::GetAssetType(handle) == AssetType::Shader) {
						m_ShaderHandle = handle;
						m_DropDownShader = -1;
						m_Material = CreateRef<Material>(m_ShaderHandle);
					}
					else {
						PE_CORE_WARN("Invalid asset type. Shader needed for material creation");
					}
				}
				ImGui::EndDragDropTarget();
			}
			ImGui::SameLine();
			ImGui::Text("Material shader");

			// Left
			{
				ImGui::BeginChild("left pane", ImVec2(150, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
				// Material edit
				if (isShaderValid) {
					const Ref<Shader> shaderAsset = AssetManager::GetAsset<Shader>(m_ShaderHandle);
					int index = 0;
					for (auto& it : m_Material->m_ShaderParameters) {
						ImGui::PushID(index);
						switch (it.second->GetType()) {
							case ShaderParameterType::UBO:
							{
								UBOShaderParameterTypeStorage* ubo = dynamic_cast<UBOShaderParameterTypeStorage*>(it.second.get());
								DrawUBOEdit(it.first, *ubo);
								break;
							}
							case ShaderParameterType::Sampler2D:
							{
								Sampler2DShaderParameterTypeStorage* sampler2D = dynamic_cast<Sampler2DShaderParameterTypeStorage*>(it.second.get());
								DrawSampler2DEdit(it.first, *sampler2D);
								break;
							}
							case ShaderParameterType::Sampler2DArray:
							{
								Sampler2DArrayShaderParameterTypeStorage* sampler2Darray = dynamic_cast<Sampler2DArrayShaderParameterTypeStorage*>(it.second.get());
								DrawSampler2DArrayEdit(it.first, *sampler2Darray);
								break;
							}
						}
						ImGui::PopID();
						index++;
					}
				}
				ImGui::EndChild();
			}
			ImGui::SameLine();
			// Right
			{
				ImGui::BeginGroup();
				ImGui::BeginChild("preview", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
				ImGui::SeparatorText("Material Preview");

				if (isShaderValid) {
					ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
					m_ViewportSize = glm::vec2(viewportPanelSize.x, viewportPanelSize.y);

					m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
					m_Camera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);

					Renderer::ResetStats();
					m_Framebuffer->Bind();
					RenderCommand::SetViewport({ 0.0f, 0.0f }, glm::ivec2((glm::ivec2)m_ViewportSize));
					RenderCommand::SetClearColour(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
					RenderCommand::Clear();

					Renderer::BeginScene(m_Camera, glm::mat4(1.0f));
					Renderer::PointLight pointLight;
					pointLight.Position = glm::vec4(0.0f, 0.0f, 0.0f, 25.0f);
					Renderer::SubmitPointLightSource(pointLight);
					Renderer::DrawDefaultCubeImmediate(m_Material, s_CubeTransform, { DepthFunc::LEQUAL, true, true }, FaceCulling::BACK, BlendState());
					Renderer::EndScene();
					m_Framebuffer->Unbind();

					FramebufferTexture2DAttachment* texAttachment = dynamic_cast<FramebufferTexture2DAttachment*>(m_Framebuffer->GetAttachment(FramebufferAttachmentPoint::Colour0).get());
					uint32_t textureID = texAttachment->GetTexture()->GetRendererID();
					ImGui::Image(textureID, ImVec2(m_ViewportSize.x, m_ViewportSize.y), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
				}

				ImGui::EndChild();

				// Save or cancel
				if (ImGui::Button("Cancel")) {
					m_ShowWindow = false;
					m_Material = nullptr;
					m_ShaderHandle = 0;
					m_DropDownShader = -1;
				}
				ImGui::SameLine();
				if (isShaderValid && ImGui::Button("Save As...")) {
					std::string path = FileDialogs::SaveFile("Paul Engine Material (*.pmat)\0*.pmat\0");
					if (!path.empty()) {
						std::filesystem::path absoluteProjectPath = std::filesystem::absolute(Project::GetProjectDirectory());
						std::filesystem::path relativeSavePath = std::filesystem::path(path).lexically_relative(absoluteProjectPath.parent_path());

						MaterialImporter::SaveMaterial(m_Material, relativeSavePath);
						Project::GetActive()->GetEditorAssetManager()->ImportAssetFromFile(relativeSavePath.lexically_relative(Project::GetAssetDirectory()), false);
					
						m_ShowWindow = false;
						m_Material = nullptr;
						m_ShaderHandle = 0;
						m_DropDownShader = -1;
					}
				}

				ImGui::EndGroup();
			}

			ImGui::End();
		}
	}

	void CreateMaterialWindow::DrawUBOEdit(const std::string& param_name, UBOShaderParameterTypeStorage& ubo)
	{
		Ref<UniformBufferStorage> uboStorage = ubo.UBO();
		auto& layout = uboStorage->GetMembers();

		if (layout.size() > 0)
		{
			ImGui::Text(param_name.c_str());
		}
		for (const UniformBufferStorage::BufferElement& e : layout) {
			const std::string& name = e.Name;
			const ShaderDataType type = e.Type;
			switch (type) {
				case ShaderDataType::Float4:
				{
					glm::vec4 data = glm::vec4(0.0f);
					ubo.UBO()->ReadLocalDataAs(name, &data);
					if (ImGui::ColorEdit4(name.c_str(), &data[0]))
					{
						uboStorage->SetLocalData(name, data);
					}
					break;
				}
				case ShaderDataType::Float3:
				{
					glm::vec3 data = glm::vec3(0.0f);
					ubo.UBO()->ReadLocalDataAs(name, &data);
					if (ImGui::ColorEdit3(name.c_str(), &data[0]))
					{
						uboStorage->SetLocalData(name, data);
					}
					break;
				}
				case ShaderDataType::Float2:
				{
					glm::vec2 data = glm::vec2(0.0f);
					ubo.UBO()->ReadLocalDataAs(name, &data);
					if (ImGui::DragFloat2(name.c_str(), &data[0]))
					{
						uboStorage->SetLocalData(name, data);
					}
					break;
				}
				case ShaderDataType::Float:
				{
					float data = 0.0f;
					ubo.UBO()->ReadLocalDataAs(name, &data);
					if (ImGui::DragFloat(name.c_str(), &data))
					{
						uboStorage->SetLocalData(name, data);
					}
					break;
				}
				case ShaderDataType::Int:
				{
					int data = 0;
					ubo.UBO()->ReadLocalDataAs(name, &data);
					if (ImGui::DragInt(name.c_str(), &data))
					{
						uboStorage->SetLocalData(name, data);
					}
					break;
				}
			}
		}
	}

	void CreateMaterialWindow::DrawSampler2DEdit(const std::string& param_name, Sampler2DShaderParameterTypeStorage& sampler)
	{
		ImGui::Text(param_name.c_str());
		AssetHandle textureHandle = sampler.m_TextureHandle;

		std::string label = "None";
		bool isTextureValid = false;
		if (textureHandle != 0) {
			if (AssetManager::IsAssetHandleValid(textureHandle) && AssetManager::GetAssetType(textureHandle) == AssetType::Texture2D) {
				const AssetMetadata& metadata = Project::GetActive()->GetEditorAssetManager()->GetMetadata(textureHandle);
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
				if (AssetManager::GetAssetType(handle) == AssetType::Texture2D) {
					sampler.m_TextureHandle = handle;
				}
				else {
					PE_CORE_WARN("Invalid asset type. Texture2D needed for sampler2D shader parameter");
				}
			}
			ImGui::EndDragDropTarget();
		}

		if (isTextureValid) {
			ImGui::SameLine();
			ImVec2 xLabelSize = ImGui::CalcTextSize("X");
			float buttonSize = xLabelSize.y + ImGui::GetStyle().FramePadding.y * 2.0f;
			if (ImGui::Button("X", ImVec2(buttonSize, buttonSize))) {
				sampler.m_TextureHandle = 0;
			}
		}
		ImGui::SameLine();
		ImGui::Text("Texture");
		ImGui::SameLine();
		ImGui::BeginDisabled(true);
		ImGui::Text("    binding: {0}", sampler.m_Binding);
		ImGui::EndDisabled();
	}

	void CreateMaterialWindow::DrawSampler2DArrayEdit(const std::string& param_name, Sampler2DArrayShaderParameterTypeStorage& samplerArray)
	{
		ImGui::Text(param_name.c_str());

		AssetHandle textureHandle = samplerArray.m_TextureArrayHandle;

		std::string label = "None";
		bool isTextureValid = false;
		if (textureHandle != 0) {
			if (AssetManager::IsAssetHandleValid(textureHandle) && AssetManager::GetAssetType(textureHandle) == AssetType::Texture2DArray) {
				const AssetMetadata& metadata = Project::GetActive()->GetEditorAssetManager()->GetMetadata(textureHandle);
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
				if (AssetManager::GetAssetType(handle) == AssetType::Texture2DArray) {
					samplerArray.m_TextureArrayHandle = handle;
				}
				else {
					PE_CORE_WARN("Invalid asset type. Texture2DArray needed for sampler2Darray shader parameter");
				}
			}
			ImGui::EndDragDropTarget();
		}

		if (isTextureValid) {
			ImGui::SameLine();
			ImVec2 xLabelSize = ImGui::CalcTextSize("X");
			float buttonSize = xLabelSize.y + ImGui::GetStyle().FramePadding.y * 2.0f;
			if (ImGui::Button("X", ImVec2(buttonSize, buttonSize))) {
				samplerArray.m_TextureArrayHandle = 0;
			}
		}
		ImGui::SameLine();
		ImGui::Text("Texture Array");
		ImGui::SameLine();
		ImGui::BeginDisabled(true);
		ImGui::Text("    binding: {0}", samplerArray.m_Binding);
		ImGui::EndDisabled();
	}
}