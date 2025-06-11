#include "pepch.h"
#include "CreateMaterialWindow.h"

#include "PaulEngine/Renderer/Renderer.h"
#include "PaulEngine/Renderer/RenderCommand.h"

#include "PaulEngine/Asset/AssetManager.h"
#include "PaulEngine/Asset/MaterialImporter.h"
#include "PaulEngine/Asset/TextureImporter.h"
#include "PaulEngine/Renderer/Asset/Material.h"

#include "PaulEngine/Utils/PlatformUtils.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <glm/glm.hpp>
#include <imgui_internal.h>

namespace PaulEngine
{
	static glm::mat4 s_MeshTransform = glm::mat4(1.0f);
	static glm::vec3 s_LightColour = glm::vec3(1.0f);
	static ImVec4 s_IconTintColourSelected = ImVec4(37.0f / 255.0f, 122.0f / 255.0f, 253.0f / 255.0f, 1.0f);
	static ImVec4 s_IconTintColour = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	static float s_LightIntensity = 1.0f;
	CreateMaterialWindow::CreateMaterialWindow(AssetHandle shaderHandle) : m_ShaderHandle(shaderHandle), m_ViewportSize(128.0f, 128.0f) {}

	void CreateMaterialWindow::Init()
	{
		m_IconSphere = TextureImporter::LoadTexture2D("Resources/Icons/mingcute--shadow-fill.png");
		m_IconCube = TextureImporter::LoadTexture2D("Resources/Icons/mingcute--box-3-fill.png");

		FramebufferSpecification spec;
		spec.Width = 1280;
		spec.Height = 720;
		spec.Samples = 1;

		TextureSpecification texSpec;
		texSpec.Width = 1280;
		texSpec.Height = 720;
		texSpec.GenerateMips = false;
		texSpec.Format = ImageFormat::RGBA16F;
		texSpec.MinFilter = ImageMinFilter::NEAREST;
		texSpec.MagFilter = ImageMagFilter::NEAREST;
		texSpec.Wrap_S = ImageWrap::CLAMP_TO_BORDER;
		texSpec.Wrap_T = ImageWrap::CLAMP_TO_BORDER;
		texSpec.Wrap_R = ImageWrap::CLAMP_TO_BORDER;
		texSpec.Border = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		Ref<FramebufferTexture2DAttachment> colour0Attach = FramebufferTexture2DAttachment::Create(FramebufferAttachmentPoint::Colour0, texSpec, true);

		texSpec.Format = ImageFormat::RED_INTEGER;
		Ref<FramebufferTexture2DAttachment> entityIDAttach = FramebufferTexture2DAttachment::Create(FramebufferAttachmentPoint::Colour1, texSpec, true);

		texSpec.Format = ImageFormat::Depth24Stencil8;
		Ref<FramebufferTexture2DAttachment> depthAttach = FramebufferTexture2DAttachment::Create(FramebufferAttachmentPoint::DepthStencil, texSpec, true);

		m_Framebuffer = Framebuffer::Create(spec, { colour0Attach, entityIDAttach }, depthAttach);

		m_Camera = SceneCamera(SCENE_CAMERA_PERSPECTIVE);

		s_MeshTransform = glm::mat4(1.0f);
		s_MeshTransform = glm::translate(s_MeshTransform, glm::vec3(0.0f, 0.0f, -1.5f));
		s_MeshTransform = glm::rotate(s_MeshTransform, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
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
			nameToFilepath["DefaultLitPBR"] = engineAssetsRelativeToProjectAssets / "shaders/Renderer3D_DefaultLitPBR.glsl";

			static const char* shaderNames[] = {
				"DefaultLit",
				"DefaultLitPBR"
			};

			if (m_DropDownShader != -1) {
				label = shaderNames[m_DropDownShader];
			}
			if (ImGui::BeginCombo("##EngineShaderDropDown", label.c_str())) {

				for (int i = 0; i < 2; i++) {
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
			
			ImGui::SameLine();
			DrawMeshIcons();

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
				ImGui::ColorEdit3("Light Colour", &s_LightColour[0], ImGuiColorEditFlags_DisplayRGB);
				ImGui::DragFloat("Light Intensity", &s_LightIntensity);

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
					pointLight.Diffuse = glm::vec4(s_LightColour * s_LightIntensity, 1.0f);
					pointLight.Specular = glm::vec4(s_LightColour * s_LightIntensity, 1.0f);
					pointLight.Ambient = glm::vec4((s_LightColour * s_LightIntensity) * 0.1f, 1.0f);
					Renderer::SubmitPointLightSource(pointLight);

					if (m_SphereSelected)
					{
						Renderer::DrawDefaultSphereImmediate(m_Material, s_MeshTransform, { DepthFunc::LEQUAL, true, true }, FaceCulling::BACK, BlendState());
					}
					else
					{
						Renderer::DrawDefaultCubeImmediate(m_Material, s_MeshTransform, { DepthFunc::LEQUAL, true, true }, FaceCulling::BACK, BlendState());
					}
					Renderer::EndScene();
					m_Framebuffer->Unbind();

					FramebufferTexture2DAttachment* texAttachment = dynamic_cast<FramebufferTexture2DAttachment*>(m_Framebuffer->GetAttachment(FramebufferAttachmentPoint::Colour0).get());
					Ref<Texture2D> textureAsset = texAttachment->GetTexture();
					uint32_t textureID = 0;
					if (textureAsset)
					{
						textureID = textureAsset->GetRendererID();
					}
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

	void CreateMaterialWindow::DrawMeshIcons()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		auto& colours = ImGui::GetStyle().Colors;
		auto& hovered = colours[ImGuiCol_ButtonHovered];
		auto& active = colours[ImGuiCol_ButtonActive];
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(hovered.x, hovered.y, hovered.z, 0.5f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(active.x, active.y, active.z, 0.5f));

		float size = ImGui::GetFrameHeight() * 1.2f;
		if (ImGui::ImageButton("##sphere_icon", m_IconSphere->GetRendererID(), ImVec2(size, size), ImVec2(0, 1), ImVec2(1, 0), ImVec4(0.0f, 0.0f, 0.0f, 0.0f), m_SphereSelected ? s_IconTintColourSelected : s_IconTintColour)) {
			m_SphereSelected = true;
		}
		ImGui::SetItemTooltip("Sphere preview");

		ImGui::SameLine();

		if (ImGui::ImageButton("##cube_icon", m_IconCube->GetRendererID(), ImVec2(size, size), ImVec2(0, 1), ImVec2(1, 0), ImVec4(0.0f, 0.0f, 0.0f, 0.0f), !m_SphereSelected ? s_IconTintColourSelected : s_IconTintColour)) {
			m_SphereSelected = false;
		}
		ImGui::SetItemTooltip("Cube preview");

		ImGui::PopStyleVar(1);
		ImGui::PopStyleColor(3);
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
					if (ImGui::DragFloat2(name.c_str(), &data[0], 0.1f))
					{
						uboStorage->SetLocalData(name, data);
					}
					break;
				}
				case ShaderDataType::Float:
				{
					float data = 0.0f;
					ubo.UBO()->ReadLocalDataAs(name, &data);
					if (ImGui::DragFloat(name.c_str(), &data, 0.01f))
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
		AssetHandle textureHandle = sampler.TextureHandle;

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
					sampler.TextureHandle = handle;
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
				sampler.TextureHandle = 0;
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

		AssetHandle textureHandle = samplerArray.TextureArrayHandle;

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
					samplerArray.TextureArrayHandle = handle;
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
				samplerArray.TextureArrayHandle = 0;
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