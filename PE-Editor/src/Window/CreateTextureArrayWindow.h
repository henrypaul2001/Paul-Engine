#pragma once
#include <PaulEngine/Asset/Asset.h>
#include <PaulEngine/Asset/TextureImporter.h>
#include <PaulEngine/Renderer/Texture.h>
#include <PaulEngine/Renderer/Framebuffer.h>
#include <PaulEngine/Scene/SceneCamera.h>

namespace PaulEngine
{
	class CreateTextureArrayWindow
	{
	public:
		CreateTextureArrayWindow();

		void OnImGuiRender();

		void SetShowWindow(bool showWindow) { m_ShowWindow = showWindow; }
		bool ShouldShowWindow() const { return m_ShowWindow; }

	private:
		bool m_ShowWindow = false;

		std::vector<Buffer> m_Buffers;
		std::vector<std::string> m_ImageNames;
		TextureImporter::ImageFileReadResult m_CurrentConstraint;
		Ref<Texture2D> m_PreviewTexture;

		Ref<Framebuffer> m_Framebuffer;
		glm::vec2 m_ViewportSize;
		SceneCamera m_Camera;

		glm::mat4 m_QuadTransform;

		// Test texture array file read
		//TextureImporter::ImageFileReadResult result;
		//Buffer albedoBuffer = TextureImporter::ReadImageFile("assets/textures/albedo.png", result);
		//Buffer aoBuffer = TextureImporter::ReadImageFile("assets/textures/ao.png", result);
		//Buffer heightBuffer = TextureImporter::ReadImageFile("assets/textures/height.png", result);
		//Buffer normalBuffer = TextureImporter::ReadImageFile("assets/textures/normal.png", result);
		//Buffer roughnessBuffer = TextureImporter::ReadImageFile("assets/textures/roughness.png", result);
		//Buffer metallicBuffer = TextureImporter::ReadImageFile("assets/textures/metallic.png", result);
		//
		//TextureSpecification spec;
		//spec.Format = ImageFormat::RGB8;
		//spec.Width = result.Width;
		//spec.Height = result.Height;
		//
		//TextureImporter::SaveBTAFile("assets/test/textureArray.bta", { albedoBuffer, normalBuffer, roughnessBuffer, metallicBuffer, aoBuffer, heightBuffer }, spec);
		//
		//s_RenderData.TestTextureArray = Texture2DArray::Create(spec, { albedoBuffer, normalBuffer, roughnessBuffer, metallicBuffer, aoBuffer, heightBuffer });
		//albedoBuffer.Release();
		//aoBuffer.Release();
		//heightBuffer.Release();
		//normalBuffer.Release();
		//roughnessBuffer.Release();
		//metallicBuffer.Release();

		//s_RenderData.TestTextureArray = TextureImporter::ReadBTAFile("assets/test/textureArray.bta");
	};
}