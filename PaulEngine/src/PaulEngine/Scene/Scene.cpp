#include "pepch.h"
#include "Scene.h"
#include "PaulEngine/Renderer/Renderer2D.h"
#include "PaulEngine/Scene/Entity.h"
namespace PaulEngine
{
	Scene::Scene() {}

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
}