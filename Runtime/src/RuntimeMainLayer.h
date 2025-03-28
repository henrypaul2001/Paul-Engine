#pragma once
#include <PaulEngine.h>

class RuntimeMainLayer : public PaulEngine::Layer
{
public:
	RuntimeMainLayer();
	~RuntimeMainLayer();

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnUpdate(const PaulEngine::Timestep timestep) override;
	virtual void OnImGuiRender() override;
	virtual void OnEvent(PaulEngine::Event& e) override;

private:
	bool OnWindowClose(PaulEngine::WindowCloseEvent& e);
	bool OnWindowResize(PaulEngine::WindowResizeEvent& e);
	bool OpenProject(const std::filesystem::path& filepath);
	bool OpenScene(PaulEngine::AssetHandle handle);

	PaulEngine::Ref<PaulEngine::Scene> m_ActiveScene;
	glm::ivec2 m_ViewportSize;
};