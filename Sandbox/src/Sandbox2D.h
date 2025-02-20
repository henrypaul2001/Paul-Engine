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
	// Temp
	PaulEngine::Ref<PaulEngine::VertexArray> m_SquareVertexArray;

	PaulEngine::ShaderLibrary m_ShaderLibrary;
	PaulEngine::OrthographicCameraController m_CameraController;

	glm::vec4 m_SquareColour = glm::vec4(0.8f, 0.2f, 0.3f, 1.0f);
};