#include "pepch.h"
#include "Scene.h"
#include "PaulEngine/Renderer/Renderer2D.h"
#include "PaulEngine/Scene/Entity.h"
#include "Components.h"

#include <box2d/box2d.h>

namespace PaulEngine
{
	namespace SceneUtils {
		static b2BodyType PEBodyType_To_Box2DBodyType(ComponentRigidBody2D::BodyType bodyType) {
			switch (bodyType) {
				case ComponentRigidBody2D::BodyType::Static:
					return b2_staticBody;
				case ComponentRigidBody2D::BodyType::Dynamic:
					return b2_dynamicBody;
				case ComponentRigidBody2D::BodyType::Kinematic:
					return b2_kinematicBody;
			}
			PE_CORE_ASSERT(false, "Unknown RB2D body type!");
			return (b2BodyType)0;
		}
	}

	Scene::Scene() : m_ViewportWidth(0), m_ViewportHeight(0) {
		m_PhysicsWorld = new b2WorldId();
	}

	Scene::~Scene() {
		if (m_PhysicsWorld) { delete m_PhysicsWorld; }
	}

	template<typename Component>
	static void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& entityMap)
	{
		auto view = src.view<Component>();
		for (auto entityID : view) {
			UUID uuid = src.get<ComponentID>(entityID).ID;
			PE_CORE_ASSERT(entityMap.find(uuid) != entityMap.end(), "Entity UUID already exists");
			entt::entity dstEntityID = entityMap.at(uuid);

			auto& component = src.get<Component>(entityID);
			dst.emplace_or_replace<Component>(dstEntityID, component);
		}
	}

	template<typename Component>
	static void CopyComponentIfExists(Entity dst, Entity src)
	{
		if (src.HasComponent<Component>()) {
			dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
		}
	}

	Ref<Scene> Scene::Copy(Ref<Scene> other)
	{
		Ref<Scene> newScene = CreateRef<Scene>();

		newScene->m_ViewportWidth = other->m_ViewportWidth;
		newScene->m_ViewportHeight = other->m_ViewportHeight;

		entt::registry& srcSceneRegistry = other->m_Registry;
		entt::registry& dstSceneRegistry = newScene->m_Registry;
		std::unordered_map<UUID, entt::entity> entityMap;

		auto idView = srcSceneRegistry.view<ComponentID>();
		for (auto entityID : idView) {
			UUID srcUUID = srcSceneRegistry.get<ComponentID>(entityID).ID;
			const std::string& tag = srcSceneRegistry.get<ComponentTag>(entityID).Tag;
			Entity newEntity = newScene->CreateEntityWithUUID(srcUUID, tag);
			entityMap[srcUUID] = (entt::entity)newEntity;
		}

		// Copy components
		CopyComponent<ComponentTransform>(dstSceneRegistry, srcSceneRegistry, entityMap);
		CopyComponent<Component2DSprite>(dstSceneRegistry, srcSceneRegistry, entityMap);
		CopyComponent<Component2DCircle>(dstSceneRegistry, srcSceneRegistry, entityMap);
		CopyComponent<ComponentCamera>(dstSceneRegistry, srcSceneRegistry, entityMap);
		CopyComponent<ComponentNativeScript>(dstSceneRegistry, srcSceneRegistry, entityMap);
		CopyComponent<ComponentRigidBody2D>(dstSceneRegistry, srcSceneRegistry, entityMap);
		CopyComponent<ComponentBoxCollider2D>(dstSceneRegistry, srcSceneRegistry, entityMap);
		CopyComponent<ComponentCircleCollider2D>(dstSceneRegistry, srcSceneRegistry, entityMap);

		return newScene;
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		return CreateEntityWithUUID(UUID(), name);
	}

	Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name)
	{
		PE_PROFILE_FUNCTION();
		Entity e = Entity(m_Registry.create(), this);
		e.AddComponent<ComponentID>(uuid);
		e.AddComponent<ComponentTransform>();
		e.AddComponent<ComponentTag>(name);
		return e;
	}

	Entity Scene::DuplicateEntity(Entity entity)
	{
		const std::string& name = entity.Tag();
		Entity newEntity = CreateEntity(name);

		CopyComponentIfExists<ComponentTransform>(newEntity, entity);
		CopyComponentIfExists<Component2DSprite>(newEntity, entity);
		CopyComponentIfExists<Component2DCircle>(newEntity, entity);
		CopyComponentIfExists<ComponentCamera>(newEntity, entity);
		CopyComponentIfExists<ComponentNativeScript>(newEntity, entity);
		CopyComponentIfExists<ComponentRigidBody2D>(newEntity, entity);
		CopyComponentIfExists<ComponentBoxCollider2D>(newEntity, entity);
		CopyComponentIfExists<ComponentCircleCollider2D>(newEntity, entity);

		return newEntity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity);
	}

	Entity Scene::GetPrimaryCameraEntity()
	{
		auto view = m_Registry.view<ComponentCamera>();
		for (auto entityID : view) {
			const auto& camera = view.get<ComponentCamera>(entityID);
			// if (camera.Primary)
			return Entity(entityID, this);
		}
		return Entity();
	}

	void Scene::OnRuntimeStart()
	{
		OnPhysics2DStart();
	}

	void Scene::OnRuntimeStop()
	{
		OnPhysics2DStop();
	}

	void Scene::OnSimulationStart()
	{
		OnPhysics2DStart();
	}

	void Scene::OnSimulationStop()
	{
		OnPhysics2DStop();
	}

	void Scene::OnPhysics2DStart()
	{
		b2WorldDef worldDefinition = b2DefaultWorldDef();
		worldDefinition.gravity = { 0.0f, -9.8f };
		*m_PhysicsWorld = b2CreateWorld(&worldDefinition);

		// Setup physics bodies
		auto group = m_Registry.group<ComponentRigidBody2D>(entt::get<ComponentTransform>);
		for (auto entityID : group) {
			Entity entity = Entity(entityID, this);
			auto [rb2d, transform] = group.get<ComponentRigidBody2D, ComponentTransform>(entityID);

			b2BodyDef bodyDefinition = b2DefaultBodyDef();
			bodyDefinition.type = SceneUtils::PEBodyType_To_Box2DBodyType(rb2d.Type);
			bodyDefinition.fixedRotation = rb2d.FixedRotation;
			bodyDefinition.position = { transform.Position.x, transform.Position.y };
			bodyDefinition.rotation = b2MakeRot(transform.Rotation.z);
			bodyDefinition.userData = (void*)&transform;	// <---	I didn't have high confidence in this working and I was right, 
			//		it doesn't work as it's not guaranteed that the reference inside 
			//		of the entt::registry will still be valid later

			b2BodyId b2Body = b2CreateBody(*m_PhysicsWorld, &bodyDefinition);
			rb2d.RuntimeBody.generation = b2Body.generation;
			rb2d.RuntimeBody.index1 = b2Body.index1;
			rb2d.RuntimeBody.world0 = b2Body.world0;

			if (entity.HasComponent<ComponentBoxCollider2D>()) {
				ComponentBoxCollider2D& bc2d = entity.GetComponent<ComponentBoxCollider2D>();
				b2Polygon box = b2MakeBox(transform.Scale.x * bc2d.Size.x, transform.Scale.y * bc2d.Size.y);
				b2ShapeDef shapeDef = b2DefaultShapeDef();

				shapeDef.density = bc2d.Density;
				shapeDef.friction = bc2d.Friction;
				shapeDef.restitution = bc2d.Restitution;

				b2ShapeId shapeId = b2CreatePolygonShape(b2Body, &shapeDef, &box);
				bc2d.RuntimeFixture.generation = shapeId.generation;
				bc2d.RuntimeFixture.index1 = shapeId.index1;
				bc2d.RuntimeFixture.world0 = shapeId.world0;
			}

			if (entity.HasComponent<ComponentCircleCollider2D>()) {
				ComponentCircleCollider2D& cc2d = entity.GetComponent<ComponentCircleCollider2D>();

				b2Circle circle;
				circle.center = { cc2d.Offset.x, cc2d.Offset.y };
				float largestScaleFactor = (transform.Scale.x > transform.Scale.y) ? transform.Scale.x : transform.Scale.y;
				circle.radius = cc2d.Radius * largestScaleFactor;

				b2ShapeDef shapeDef = b2DefaultShapeDef();
				shapeDef.density = cc2d.Density;
				shapeDef.friction = cc2d.Friction;
				shapeDef.restitution = cc2d.Restitution;

				b2ShapeId shapeId = b2CreateCircleShape(b2Body, &shapeDef, &circle);
				cc2d.RuntimeFixture.generation = shapeId.generation;
				cc2d.RuntimeFixture.index1 = shapeId.index1;
				cc2d.RuntimeFixture.world0 = shapeId.world0;
			}
		}
	}

	void Scene::OnPhysics2DStop()
	{
		b2DestroyWorld(*m_PhysicsWorld);
		delete m_PhysicsWorld;
		m_PhysicsWorld = nullptr;
	}

	void Scene::RenderScene(EditorCamera& camera)
	{
		Renderer2D::BeginScene(camera);
		{
			PE_PROFILE_SCOPE("Draw Quads");
			auto group = m_Registry.group<ComponentTransform>(entt::get<Component2DSprite>);
			for (auto entityID : group) {
				auto [transform, sprite] = group.get<ComponentTransform, Component2DSprite>(entityID);
				if (sprite.Texture) {
					Renderer2D::DrawQuad(transform.GetTransform(), sprite.Texture, sprite.TextureScale, sprite.Colour, (int)entityID);
				}
				else {
					Renderer2D::DrawQuad(transform.GetTransform(), sprite.Colour, (int)entityID);
				}
			}
		}

		{
			PE_PROFILE_SCOPE("Draw Circles");
			auto view = m_Registry.view<ComponentTransform, Component2DCircle>();
			for (auto entityID : view) {
				auto [transform, circle] = view.get<ComponentTransform, Component2DCircle>(entityID);

				Renderer2D::DrawCircle(transform.GetTransform(), circle.Colour, circle.Thickness, circle.Fade, (int)entityID);
			}
		}

		{
			PE_PROFILE_SCOPE("Draw Text");
			auto view = m_Registry.view<ComponentTransform, ComponentTextRenderer>();
			for (auto entityID : view) {
				auto [transform, text] = view.get<ComponentTransform, ComponentTextRenderer>(entityID);

				Renderer2D::TextParams params;
				params.Colour = text.Colour;
				params.Kerning = text.Kerning;
				params.LineSpacing = text.LineSpacing;

				Renderer2D::DrawString(text.TextString, text.FontAsset, transform.GetTransform(), params, (int)entityID);
			}
		}

		Renderer2D::EndScene();
	}

	void Scene::OnUpdateRuntime(Timestep timestep)
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

		// Physics
		{
			PE_PROFILE_SCOPE("Box2D Physics");
			const int32_t velocityIterations = 6;
			const int32_t positionIteratiorns = 2;

			b2World_Step(*m_PhysicsWorld, timestep, 4);

			//b2BodyEvents events = b2World_GetBodyEvents(*m_PhysicsWorld);
			//for (int i = 0; i < events.moveCount; i++) {
			//	const b2BodyMoveEvent* event = events.moveEvents + i;
			//	ComponentTransform* transformPtr = static_cast<ComponentTransform*>(event->userData);
			//
			//	transformPtr->Position.x = event->transform.p.x;
			//	transformPtr->Position.y = event->transform.p.y;
			//	transformPtr->Rotation.z = b2Rot_GetAngle(event->transform.q);
			//}

			auto view = m_Registry.view<ComponentRigidBody2D>();
			for (auto entityID : view) {
				Entity entity = Entity(entityID, this);
				auto& transform = entity.GetComponent<ComponentTransform>();
				auto& rb2d = entity.GetComponent<ComponentRigidBody2D>();

				b2BodyId body = {
					rb2d.RuntimeBody.index1,
					rb2d.RuntimeBody.world0,
					rb2d.RuntimeBody.generation
				};
				b2Transform box2DTransform = b2Body_GetTransform(body);
				transform.Position.x = box2DTransform.p.x;
				transform.Position.y = box2DTransform.p.y;
				transform.Rotation.z = b2Rot_GetAngle(box2DTransform.q);
			}
		}

		// Render 2D
		Entity cameraEntity = GetPrimaryCameraEntity();
		if (cameraEntity) {
			Camera& mainCamera = cameraEntity.GetComponent<ComponentCamera>().Camera;
			Renderer2D::BeginScene(mainCamera, cameraEntity.GetComponent<ComponentTransform>().GetTransform());
			{
				PE_PROFILE_SCOPE("Draw Quads");
				auto group = m_Registry.group<ComponentTransform>(entt::get<Component2DSprite>);
				for (auto entityID : group) {
					auto [transform, sprite] = group.get<ComponentTransform, Component2DSprite>(entityID);
					if (sprite.Texture) {
						Renderer2D::DrawQuad(transform.GetTransform(), sprite.Texture, sprite.TextureScale, sprite.Colour, (int)entityID);
					}
					else {
						Renderer2D::DrawQuad(transform.GetTransform(), sprite.Colour, (int)entityID);
					}
				}
			}

			{
				PE_PROFILE_SCOPE("Draw Circles");
				auto view = m_Registry.view<ComponentTransform, Component2DCircle>();
				for (auto entityID : view) {
					auto [transform, circle] = view.get<ComponentTransform, Component2DCircle>(entityID);

					Renderer2D::DrawCircle(transform.GetTransform(), circle.Colour, circle.Thickness, circle.Fade, (int)entityID);
				}
			}
			Renderer2D::EndScene();
		}
	}

	void Scene::OnUpdateSimulation(Timestep timestep, EditorCamera& camera)
	{
		// Physics
		{
			PE_PROFILE_SCOPE("Box2D Physics");
			const int32_t velocityIterations = 6;
			const int32_t positionIteratiorns = 2;

			b2World_Step(*m_PhysicsWorld, timestep, 4);

			//b2BodyEvents events = b2World_GetBodyEvents(*m_PhysicsWorld);
			//for (int i = 0; i < events.moveCount; i++) {
			//	const b2BodyMoveEvent* event = events.moveEvents + i;
			//	ComponentTransform* transformPtr = static_cast<ComponentTransform*>(event->userData);
			//
			//	transformPtr->Position.x = event->transform.p.x;
			//	transformPtr->Position.y = event->transform.p.y;
			//	transformPtr->Rotation.z = b2Rot_GetAngle(event->transform.q);
			//}

			auto view = m_Registry.view<ComponentRigidBody2D>();
			for (auto entityID : view) {
				Entity entity = Entity(entityID, this);
				auto& transform = entity.GetComponent<ComponentTransform>();
				auto& rb2d = entity.GetComponent<ComponentRigidBody2D>();

				b2BodyId body = {
					rb2d.RuntimeBody.index1,
					rb2d.RuntimeBody.world0,
					rb2d.RuntimeBody.generation
				};
				b2Transform box2DTransform = b2Body_GetTransform(body);
				transform.Position.x = box2DTransform.p.x;
				transform.Position.y = box2DTransform.p.y;
				transform.Rotation.z = b2Rot_GetAngle(box2DTransform.q);
			}
		}

		RenderScene(camera);
	}

	void Scene::OnUpdateOffline(Timestep timestep, EditorCamera& camera)
	{
		RenderScene(camera);
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