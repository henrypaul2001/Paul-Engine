#pragma once
#include <PaulEngine.h>
#include <PaulEngine/OrthographicCameraController.h>

class Sandbox2D : public PaulEngine::Layer
{
public:
	Sandbox2D();
	~Sandbox2D();

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnUpdate(const PaulEngine::Timestep timestep) override;
	virtual void OnImGuiRender() override;
	virtual void OnEvent(PaulEngine::Event& e) override;

private:
	bool OnKeyUp(PaulEngine::KeyReleasedEvent& e);

	// Temp
	PaulEngine::Ref<PaulEngine::VertexArray> m_SquareVertexArray;

	PaulEngine::ShaderLibrary m_ShaderLibrary;
	PaulEngine::OrthographicCameraController m_CameraController;
	PaulEngine::Ref<PaulEngine::Texture2D> m_Texture, m_Texture2;

	PaulEngine::Ref<PaulEngine::Texture2D> m_Spritesheet;
	PaulEngine::Ref<PaulEngine::SubTexture2D> m_TextureStairs, m_TextureBarrel, m_TextureTree, m_TextureInvalid;

	glm::vec4 m_SquareColour = glm::vec4(0.8f, 0.2f, 0.3f, 1.0f);

	uint32_t m_MapWidth, m_MapHeight;
	std::unordered_map<char, PaulEngine::Ref<PaulEngine::SubTexture2D>> m_TextureMap;
};