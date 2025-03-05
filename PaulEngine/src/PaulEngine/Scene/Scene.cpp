#include "pepch.h"
#include "Scene.h"
#include "PaulEngine/Renderer/Renderer2D.h"
#include "PaulEngine/Scene/Entity.h"
#include "Components.h"

namespace PaulEngine
{
	Scene::Scene() : m_ViewportWidth(0), m_ViewportHeight(0) {}

	Scene::~Scene() {}

	Entity Scene::CreateEntity(const std::string& name)
	{
		PE_PROFILE_FUNCTION();
		Entity e = Entity(m_Registry.create(), this);
		e.AddComponent<ComponentTransform>();
		e.AddComponent<ComponentTag>(name);
		return e;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity);
	}

	void Scene::OnUpdate(Timestep timestep)
	{
		PE_PROFILE_FUNCTION();

		// Update scripts
		{
			PE_PROFILE_SCOPE("Run scripts");
			m_Registry.view<ComponentNativeScript>().each([=](auto entity, auto& script) {
				// Script instantiation will be moved to OnSceneStart
				if (!script.Instance) {
					script.Instance = script.InstantiateScript();
					script.Instance->m_Entity = Entity(entity, this);
					script.Instance->OnCreate();
				}

				script.Instance->OnUpdate(timestep);
			});
		}

		Camera* mainCamera = nullptr;
		glm::mat4 transformation = glm::mat4(1.0f);
		{
			auto group = m_Registry.group<ComponentCamera>(entt::get<ComponentTransform>);
			for (auto entity : group) {
				auto [transform, camera] = group.get<ComponentTransform, ComponentCamera>(entity);

				// if camera.primary
				mainCamera = &camera.Camera;
				transformation = transform.GetTransform();
				break;
			}
		}

		if (mainCamera) {
			Renderer2D::BeginScene(*mainCamera, transformation);
			auto group = m_Registry.group<ComponentTransform>(entt::get<Component2DSprite>);
			for (auto entity : group) {
				auto [transform, sprite] = group.get<ComponentTransform, Component2DSprite>(entity);
				Renderer2D::DrawQuad(transform.GetTransform(), sprite.Colour);
			}
			Renderer2D::EndScene();
		}
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		PE_PROFILE_FUNCTION();
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