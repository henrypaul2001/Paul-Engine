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
		auto group = m_Registry.group<ComponentTransform>(entt::get<Component2DSprite>);
		for (auto entity : group) {
			auto& [transform, sprite] = group.get<ComponentTransform, Component2DSprite>(entity);
			Renderer2D::DrawQuad(transform.Position, glm::vec2(transform.Scale.x, transform.Scale.y), sprite.Colour);
		}
	}
}