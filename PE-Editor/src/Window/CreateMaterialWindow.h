#pragma once
#include <PaulEngine/Asset/Asset.h>
#include <PaulEngine/Renderer/Resource/Framebuffer.h>
#include <PaulEngine/Scene/SceneCamera.h>

#include <PaulEngine/Renderer/Asset/Material.h>

namespace PaulEngine
{
	class CreateMaterialWindow
	{
	public:
		CreateMaterialWindow(AssetHandle shaderHandle);
		
		void SetContext(AssetHandle shaderHandle);

		void OnImGuiRender();

		void SetShowWindow(bool showWindow) { m_ShowWindow = showWindow; }
		bool ShouldShowWindow() const { return m_ShowWindow; }

		void Init();
	private:
		void DrawMeshIcons();
		void DrawUBOEdit(const std::string& param_name, UBOShaderParameterTypeStorage& ubo);
		void DrawSampler2DEdit(const std::string& param_name, Sampler2DShaderParameterTypeStorage& sampler);
		void DrawSampler2DArrayEdit(const std::string& param_name, Sampler2DArrayShaderParameterTypeStorage& samplerArray);

		Ref<Texture2D> m_IconSphere;
		Ref<Texture2D> m_IconCube;
		bool m_SphereSelected = true;

		AssetHandle m_ShaderHandle;
		Ref<Material> m_Material;

		bool m_ShowWindow = false;

		Ref<Framebuffer> m_Framebuffer;

		glm::vec2 m_ViewportSize;
		SceneCamera m_Camera;

		int m_DropDownShader = -1;
	};
}