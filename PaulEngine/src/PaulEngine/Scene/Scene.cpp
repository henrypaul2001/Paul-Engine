#include "pepch.h"
#include "Scene.h"
#include "PaulEngine/Renderer/Renderer2D.h"
#include "PaulEngine/Scene/Entity.h"
namespace PaulEngine
{
	Scene::Scene() : m_ViewportWidth(0), m_ViewportHeight(0) {}

	Scene::~Scene() {}

	Entity Scene::CreateEntity(const std::string& name)
	{
		Entity e = Entity(m_Registry.create(), this);
		e.AddComponent<ComponentTransform>();
		e.AddComponent<ComponentTag>(name);
		return e;
	}

	void Scene::OnUpdate(Timestep timestep)
	{
		Camera* mainCamera = nullptr;
		glm::mat4 transformation = glm::mat4(1.0f);
		{
			auto group = m_Registry.group<ComponentCamera>(entt::get<ComponentTransform>);
			for (auto entity : group) {
				auto& [transform, camera] = group.get<ComponentTransform, ComponentCamera>(entity);

				// if camera.primary
				mainCamera = &camera.Camera;
				transformation = glm::translate(transformation, transform.Position);
				transformation = glm::scale(transformation, transform.Scale);
				break;
			}
		}

		if (mainCamera) {
			Renderer2D::BeginScene(*mainCamera, transformation);
			auto group = m_Registry.group<ComponentTransform>(entt::get<Component2DSprite>);
			for (auto entity : group) {
				auto& [transform, sprite] = group.get<ComponentTransform, Component2DSprite>(entity);
				Renderer2D::DrawQuad(transform.Position, glm::vec2(transform.Scale.x, transform.Scale.y), sprite.Colour);
			}
			Renderer2D::EndScene();
		}
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;

		// Resize non fixed aspect ratio cameras
		auto view = m_Registry.view<ComponentCamera>();
		for (auto entity : view) {
			auto& cameraComponent = view.get<ComponentCamera>(entity);
			if (!cameraComponent.FixedAspectRatio) {
				cameraComponent.Camera.SetViewportSize(width, height);
			}
		}
	}
}