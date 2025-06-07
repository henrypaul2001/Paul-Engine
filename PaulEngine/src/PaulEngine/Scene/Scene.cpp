#include "pepch.h"
#include "Scene.h"
#include "PaulEngine/Renderer/Renderer.h"
#include "PaulEngine/Renderer/Renderer2D.h"
#include "PaulEngine/Scene/Entity.h"
#include "Components.h"
#include "PaulEngine/Scene/Prefab.h"
#include "PaulEngine/Asset/AssetManager.h"

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
		PE_PROFILE_FUNCTION();
		Ref<Scene> newScene = CreateRef<Scene>();

		newScene->m_ViewportWidth = other->m_ViewportWidth;
		newScene->m_ViewportHeight = other->m_ViewportHeight;

		newScene->Append(other);

		return newScene;
	}

	void Scene::Append(Ref<Scene> source)
	{
		PE_PROFILE_FUNCTION();
		entt::registry& srcSceneRegistry = source->m_Registry;
		entt::registry& dstSceneRegistry = m_Registry;
		std::unordered_map<UUID, entt::entity> entityMap;

		auto idView = srcSceneRegistry.view<ComponentID>();
		for (auto entityID : idView) {
			UUID srcUUID = srcSceneRegistry.get<ComponentID>(entityID).ID;
			const std::string& tag = srcSceneRegistry.get<ComponentTag>(entityID).Tag;
			Entity newEntity = CreateEntityWithUUID(srcUUID, tag);
			entityMap[srcUUID] = (entt::entity)newEntity;
		}

		// Copy components
		CopyComponent<ComponentPrefabSource>(dstSceneRegistry, srcSceneRegistry, entityMap);
		CopyComponent<ComponentTransform>(dstSceneRegistry, srcSceneRegistry, entityMap);
		CopyComponent<Component2DSprite>(dstSceneRegistry, srcSceneRegistry, entityMap);
		CopyComponent<Component2DCircle>(dstSceneRegistry, srcSceneRegistry, entityMap);
		CopyComponent<ComponentCamera>(dstSceneRegistry, srcSceneRegistry, entityMap);
		CopyComponent<ComponentNativeScript>(dstSceneRegistry, srcSceneRegistry, entityMap);
		CopyComponent<ComponentRigidBody2D>(dstSceneRegistry, srcSceneRegistry, entityMap);
		CopyComponent<ComponentBoxCollider2D>(dstSceneRegistry, srcSceneRegistry, entityMap);
		CopyComponent<ComponentCircleCollider2D>(dstSceneRegistry, srcSceneRegistry, entityMap);
		CopyComponent<ComponentTextRenderer>(dstSceneRegistry, srcSceneRegistry, entityMap);
		CopyComponent<ComponentMeshRenderer>(dstSceneRegistry, srcSceneRegistry, entityMap);
		CopyComponent<ComponentDirectionalLight>(dstSceneRegistry, srcSceneRegistry, entityMap);
		CopyComponent<ComponentPointLight>(dstSceneRegistry, srcSceneRegistry, entityMap);
		CopyComponent<ComponentSpotLight>(dstSceneRegistry, srcSceneRegistry, entityMap);

		auto transformView = dstSceneRegistry.view<ComponentTransform>();
		for (auto entityID : transformView)
		{
			dstSceneRegistry.get<ComponentTransform>(entityID).RemapEntityRelationships(this);
		}
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
		m_EntityMap[uuid] = e;
		return e;
	}

	Entity Scene::DuplicateEntity(Entity entity)
	{
		PE_PROFILE_FUNCTION();
		const std::string& name = entity.Tag();
		Entity newEntity = CreateEntity(name);

		CopyComponentIfExists<ComponentPrefabSource>(newEntity, entity);
		CopyComponentIfExists<ComponentTransform>(newEntity, entity);
		CopyComponentIfExists<Component2DSprite>(newEntity, entity);
		CopyComponentIfExists<Component2DCircle>(newEntity, entity);
		CopyComponentIfExists<ComponentCamera>(newEntity, entity);
		CopyComponentIfExists<ComponentNativeScript>(newEntity, entity);
		CopyComponentIfExists<ComponentRigidBody2D>(newEntity, entity);
		CopyComponentIfExists<ComponentBoxCollider2D>(newEntity, entity);
		CopyComponentIfExists<ComponentCircleCollider2D>(newEntity, entity);
		CopyComponentIfExists<ComponentMeshRenderer>(newEntity, entity);
		CopyComponentIfExists<ComponentDirectionalLight>(newEntity, entity);
		CopyComponentIfExists<ComponentPointLight>(newEntity, entity);
		CopyComponentIfExists<ComponentSpotLight>(newEntity, entity);

		ComponentTransform& newTransform = newEntity.GetComponent<ComponentTransform>();
		ComponentTransform& originalTransform = entity.GetComponent<ComponentTransform>();
		
		// Inform parent of new child
		Entity parent = newTransform.GetParent();
		if (parent.IsValid())
		{
			parent.GetComponent<ComponentTransform>().m_Children.emplace(newEntity);
		}

		// Duplicate children
		std::unordered_set<Entity> children = originalTransform.GetChildren();
		newTransform.m_Children.clear();
		for (Entity e : children)
		{
			Entity newChild = DuplicateEntity(e);
			ComponentTransform::SetParent(newChild, newEntity);
		}

		return newEntity;
	}

	Entity Scene::CopyEntityToScene(Entity srcEntity, Scene* dstScene)
	{
		PE_CORE_ASSERT(srcEntity.IsValid(), "Invalid entity");
		const std::string& name = srcEntity.Tag();
		Entity newEntity = dstScene->CreateEntityWithUUID(srcEntity.UUID(), name);

		CopyComponentIfExists<ComponentPrefabSource>(newEntity, srcEntity);
		CopyComponentIfExists<ComponentTransform>(newEntity, srcEntity);
		CopyComponentIfExists<Component2DSprite>(newEntity, srcEntity);
		CopyComponentIfExists<Component2DCircle>(newEntity, srcEntity);
		CopyComponentIfExists<ComponentCamera>(newEntity, srcEntity);
		CopyComponentIfExists<ComponentNativeScript>(newEntity, srcEntity);
		CopyComponentIfExists<ComponentRigidBody2D>(newEntity, srcEntity);
		CopyComponentIfExists<ComponentBoxCollider2D>(newEntity, srcEntity);
		CopyComponentIfExists<ComponentCircleCollider2D>(newEntity, srcEntity);
		CopyComponentIfExists<ComponentMeshRenderer>(newEntity, srcEntity);
		CopyComponentIfExists<ComponentDirectionalLight>(newEntity, srcEntity);
		CopyComponentIfExists<ComponentPointLight>(newEntity, srcEntity);
		CopyComponentIfExists<ComponentSpotLight>(newEntity, srcEntity);

		ComponentTransform& newTransform = newEntity.GetComponent<ComponentTransform>();
		ComponentTransform& originalTransform = srcEntity.GetComponent<ComponentTransform>();

		// Inform parent of new child
		Entity parent = newTransform.GetParent();
		if (parent.IsValid())
		{
			parent.GetComponent<ComponentTransform>().m_Children.emplace(newEntity);
		}

		// Duplicate children
		std::unordered_set<Entity> children = originalTransform.GetChildren();
		newTransform.m_Children.clear();
		for (Entity e : children)
		{
			Entity newChild = CopyEntityToScene(e, dstScene);
			ComponentTransform::SetParent(newChild, newEntity);
		}

		return newEntity;
	}

	Entity Scene::FindEntityWithUUID(UUID id)
	{
		if (m_EntityMap.find(id) != m_EntityMap.end())
		{
			return m_EntityMap.at(id);
		}
		else
		{
			return Entity();
		}
	}

	void Scene::DestroyEntity(Entity entity)
	{
		ComponentTransform& transform = entity.GetComponent<ComponentTransform>();

		// Remove entity from parents child list
		Entity parent = transform.GetParent();
		if (parent.IsValid())
		{
			ComponentTransform& parentTransform = parent.GetComponent<ComponentTransform>();
			parentTransform.m_Children.erase(entity);
		}
		DestroyChildren(entity);
		m_EntityMap.erase(entity.GetComponent<ComponentID>().ID);
		m_Registry.destroy(entity);
	}

	void Scene::DestroyChildren(Entity rootEntity)
	{
		ComponentTransform& transform = rootEntity.GetComponent<ComponentTransform>();
		std::unordered_set<Entity> children = transform.GetChildren();
		for (Entity e : children)
		{
			DestroyChildren(e);
			m_EntityMap.erase(e.GetComponent<ComponentID>().ID);
			m_Registry.destroy(e);
		}
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

	std::vector<Entity> Scene::GetRootEntities()
	{
		std::vector<Entity> rootEntities;
		auto view = m_Registry.view<ComponentTransform>();
		for (auto entityID : view)
		{
			const auto& transform = view.get<ComponentTransform>(entityID);
			if (!transform.GetParent().IsValid())
			{
				rootEntities.push_back(Entity(entityID, this));
			}
		}
		return rootEntities;
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

	void Scene::OnPhysics2DStep(Timestep timestep)
	{
		{
			PE_PROFILE_SCOPE("Box2D Physics");
			{
				PE_PROFILE_SCOPE("Box2D: Update dirty physics objects");

				// If transform has changed, dirty all Box2D related components
				m_Registry.view<ComponentTransform>().each([this](auto entityID, ComponentTransform& transform) {
					Entity entity = Entity(entityID, this);
					if (transform.m_PhysicsDirty) {
						if (entity.HasComponent<ComponentRigidBody2D>()) {
							ComponentRigidBody2D& rb2d = entity.GetComponent<ComponentRigidBody2D>();
							b2BodyId bodyID;
							bodyID.generation = rb2d.m_RuntimeBody.generation;
							bodyID.index1 = rb2d.m_RuntimeBody.index1;
							bodyID.world0 = rb2d.m_RuntimeBody.world0;

							b2Body_SetTransform(bodyID, { transform.m_Position.x, transform.m_Position.y }, b2MakeRot(transform.m_Rotation.z));
						}
						if (entity.HasComponent<ComponentBoxCollider2D>()) {
							entity.GetComponent<ComponentBoxCollider2D>().m_PhysicsDirty = true;
						}
						if (entity.HasComponent<ComponentCircleCollider2D>()) {
							entity.GetComponent<ComponentCircleCollider2D>().m_PhysicsDirty = true;
						}
						transform.m_PhysicsDirty = false;
					}
				});

				// Rigid body 2D
				m_Registry.view<ComponentRigidBody2D>().each([this](auto entityID, ComponentRigidBody2D& rb2d) {
					if (rb2d.m_PhysicsDirty) {
						b2BodyId bodyID;
						bodyID.generation = rb2d.m_RuntimeBody.generation;
						bodyID.index1 = rb2d.m_RuntimeBody.index1;
						bodyID.world0 = rb2d.m_RuntimeBody.world0;

						b2Body_SetType(bodyID, SceneUtils::PEBodyType_To_Box2DBodyType(rb2d.m_Type));
						b2Body_SetFixedRotation(bodyID, rb2d.m_FixedRotation);

						rb2d.m_PhysicsDirty = false;
					}
				});

				// Box colliders
				m_Registry.view<ComponentBoxCollider2D>().each([this](auto entityID, ComponentBoxCollider2D& bc2d) {
					ComponentTransform& transform = Entity(entityID, this).GetComponent<ComponentTransform>();
					if (bc2d.m_PhysicsDirty) {
						b2ShapeId shapeID;
						shapeID.generation = bc2d.m_RuntimeFixture.generation;
						shapeID.index1 = bc2d.m_RuntimeFixture.index1;
						shapeID.world0 = bc2d.m_RuntimeFixture.world0;

						b2Polygon box = b2MakeBox(transform.m_Scale.x * bc2d.m_Size.x, transform.m_Scale.y * bc2d.m_Size.y);

						b2Shape_SetPolygon(shapeID, &box);

						b2Shape_SetDensity(shapeID, bc2d.m_Density, true);
						b2Shape_SetFriction(shapeID, bc2d.m_Friction);
						b2Shape_SetRestitution(shapeID, bc2d.m_Restitution);

						bc2d.m_PhysicsDirty = false;
					}
				});

				// Circle colliders
				m_Registry.view<ComponentCircleCollider2D>().each([this](auto entityID, ComponentCircleCollider2D& cc2d) {
					ComponentTransform& transform = Entity(entityID, this).GetComponent<ComponentTransform>();
					if (cc2d.m_PhysicsDirty) {
						b2ShapeId shapeID;
						shapeID.generation = cc2d.m_RuntimeFixture.generation;
						shapeID.index1 = cc2d.m_RuntimeFixture.index1;
						shapeID.world0 = cc2d.m_RuntimeFixture.world0;

						b2Circle circle;
						circle.center = { 0.0f, 0.0f };
						float largestScaleFactor = (transform.m_Scale.x > transform.m_Scale.y) ? transform.m_Scale.x : transform.m_Scale.y;
						circle.radius = cc2d.m_Radius * largestScaleFactor;

						b2Shape_SetCircle(shapeID, &circle);

						b2Shape_SetDensity(shapeID, cc2d.m_Density, true);
						b2Shape_SetFriction(shapeID, cc2d.m_Friction);
						b2Shape_SetRestitution(shapeID, cc2d.m_Restitution);

						cc2d.m_PhysicsDirty = false;
					}
				});
			}

			{
				PE_PROFILE_SCOPE("Box2D: World step");
				const int32_t velocityIterations = 6;
				const int32_t positionIteratiorns = 2;

				b2World_Step(*m_PhysicsWorld, timestep, 4);
			}

			{
				PE_PROFILE_SCOPE("Box2D: Update transform components with new, simulated values");
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
						rb2d.m_RuntimeBody.index1,
						rb2d.m_RuntimeBody.world0,
						rb2d.m_RuntimeBody.generation
					};
					b2Transform box2DTransform = b2Body_GetTransform(body);
					transform.m_Position.x = box2DTransform.p.x;
					transform.m_Position.y = box2DTransform.p.y;
					transform.m_Rotation.z = b2Rot_GetAngle(box2DTransform.q);
					transform.m_TransformDirty = true;
				}
			}
		}
	}

	void Scene::OnPhysics2DStart()
	{
		b2WorldDef worldDefinition = b2DefaultWorldDef();
		worldDefinition.gravity = { 0.0f, -9.8f };
		if (!m_PhysicsWorld) { m_PhysicsWorld = new b2WorldId(); }
		*m_PhysicsWorld = b2CreateWorld(&worldDefinition);

		// Setup physics bodies
		auto group = m_Registry.group<ComponentRigidBody2D>(entt::get<ComponentTransform>);
		for (auto entityID : group) {
			Entity entity = Entity(entityID, this);
			auto [rb2d, transform] = group.get<ComponentRigidBody2D, ComponentTransform>(entityID);

			b2BodyDef bodyDefinition = b2DefaultBodyDef();
			bodyDefinition.type = SceneUtils::PEBodyType_To_Box2DBodyType(rb2d.m_Type);
			bodyDefinition.fixedRotation = rb2d.m_FixedRotation;
			bodyDefinition.position = { transform.m_Position.x, transform.m_Position.y };
			bodyDefinition.rotation = b2MakeRot(transform.m_Rotation.z);
			bodyDefinition.userData = (void*)&transform;	// <---	I didn't have high confidence in this working and I was right, 
			//		it doesn't work as it's not guaranteed that the reference inside 
			//		of the entt::registry will still be valid later

			b2BodyId b2Body = b2CreateBody(*m_PhysicsWorld, &bodyDefinition);
			rb2d.m_RuntimeBody.generation = b2Body.generation;
			rb2d.m_RuntimeBody.index1 = b2Body.index1;
			rb2d.m_RuntimeBody.world0 = b2Body.world0;

			if (entity.HasComponent<ComponentBoxCollider2D>()) {
				ComponentBoxCollider2D& bc2d = entity.GetComponent<ComponentBoxCollider2D>();
				b2Polygon box = b2MakeBox(transform.m_Scale.x * bc2d.m_Size.x, transform.m_Scale.y * bc2d.m_Size.y);
				b2ShapeDef shapeDef = b2DefaultShapeDef();

				shapeDef.density = bc2d.m_Density;
				shapeDef.friction = bc2d.m_Friction;
				shapeDef.restitution = bc2d.m_Restitution;

				b2ShapeId shapeId = b2CreatePolygonShape(b2Body, &shapeDef, &box);
				bc2d.m_RuntimeFixture.generation = shapeId.generation;
				bc2d.m_RuntimeFixture.index1 = shapeId.index1;
				bc2d.m_RuntimeFixture.world0 = shapeId.world0;
			}

			if (entity.HasComponent<ComponentCircleCollider2D>()) {
				ComponentCircleCollider2D& cc2d = entity.GetComponent<ComponentCircleCollider2D>();

				b2Circle circle;
				circle.center = { 0.0f, 0.0f };
				float largestScaleFactor = (transform.m_Scale.x > transform.m_Scale.y) ? transform.m_Scale.x : transform.m_Scale.y;
				circle.radius = cc2d.m_Radius * largestScaleFactor;

				b2ShapeDef shapeDef = b2DefaultShapeDef();
				shapeDef.density = cc2d.m_Density;
				shapeDef.friction = cc2d.m_Friction;
				shapeDef.restitution = cc2d.m_Restitution;

				b2ShapeId shapeId = b2CreateCircleShape(b2Body, &shapeDef, &circle);
				cc2d.m_RuntimeFixture.generation = shapeId.generation;
				cc2d.m_RuntimeFixture.index1 = shapeId.index1;
				cc2d.m_RuntimeFixture.world0 = shapeId.world0;
			}
		}
	}

	void Scene::OnPhysics2DStop()
	{
		b2DestroyWorld(*m_PhysicsWorld);
		delete m_PhysicsWorld;
		m_PhysicsWorld = nullptr;
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

		OnPhysics2DStep(timestep);
	}

	void Scene::OnUpdateSimulation(Timestep timestep, EditorCamera& camera)
	{
		OnPhysics2DStep(timestep);
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