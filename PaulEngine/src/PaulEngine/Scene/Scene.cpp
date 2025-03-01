#include "pepch.h"
#include "Scene.h"
#include "PaulEngine/Renderer/Renderer2D.h"
namespace PaulEngine
{
	Scene::Scene() {}

	Scene::~Scene() {}

	entt::entity Scene::CreateEntity()
	{
		return m_Registry.create();
	}

	void Scene::OnUpdate(Timestep timestep)
	{
		auto group = m_Registry.group<ComponentTransform>(entt::get<Component2DSprite>);
		for (auto entity : group) {
			auto& [transform, sprite] = group.get<ComponentTransform, Component2DSprite>(entity);
			Renderer2D::DrawQuad(transform.Position, glm::vec2(transform.Scale.x, transform.Scale.y), sprite.Colour);
		}
	}
}