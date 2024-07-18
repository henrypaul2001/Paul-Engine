#pragma once
#include "Component.h"
#include "ComponentCollisionAABB.h"
#include "ComponentCollisionSphere.h"
#include "ComponentCollisionBox.h"
#include "ComponentPhysics.h"
#include "ComponentGeometry.h"
#include "ComponentLight.h"
#include "ComponentUICanvas.h"
#include "ComponentAnimator.h"
#include "ComponentAudioSource.h"
#include "ComponentParticleGenerator.h"
#include "ComponentPathfinder.h"
#include "ComponentStateController.h"
namespace Engine
{
	class EntityManager;

	class Entity
	{
	private:
		std::string name;
		std::vector<Component*> componentList;
		ComponentTypes mask;

		EntityManager* entityManager;

	public:
		Entity(const Entity& old_entity);
		Entity(std::string name);
		~Entity();

		std::string& Name() { return name; }
		ComponentTypes& Mask() { return mask; }
		std::vector<Component*>& Components() { return componentList; }
		Component* GetComponent(ComponentTypes type);

		Entity* Clone();

		ComponentTransform* GetTransformComponent() { return dynamic_cast<ComponentTransform*>(GetComponent(COMPONENT_TRANSFORM)); }
		ComponentGeometry* GetGeometryComponent() { return dynamic_cast<ComponentGeometry*>(GetComponent(COMPONENT_GEOMETRY)); }
		ComponentLight* GetLightComponent() { return dynamic_cast<ComponentLight*>(GetComponent(COMPONENT_LIGHT)); }
		ComponentCollisionAABB* GetAABBCollisionComponent() { return dynamic_cast<ComponentCollisionAABB*>(GetComponent(COMPONENT_COLLISION_AABB)); }
		ComponentCollisionSphere* GetSphereCollisionComponent() { return dynamic_cast<ComponentCollisionSphere*>(GetComponent(COMPONENT_COLLISION_SPHERE)); }
		ComponentCollisionBox* GetBoxCollisionComponent() { return dynamic_cast<ComponentCollisionBox*>(GetComponent(COMPONENT_COLLISION_BOX)); }
		ComponentPhysics* GetPhysicsComponent() { return dynamic_cast<ComponentPhysics*>(GetComponent(COMPONENT_PHYSICS)); }
		ComponentUICanvas* GetUICanvasComponent() { return dynamic_cast<ComponentUICanvas*>(GetComponent(COMPONENT_UICANVAS)); }
		ComponentAnimator* GetAnimator() { return dynamic_cast<ComponentAnimator*>(GetComponent(COMPONENT_ANIMATOR)); }
		ComponentAudioSource* GetAudioComponent() { return dynamic_cast<ComponentAudioSource*>(GetComponent(COMPONENT_AUDIO_SOURCE)); }
		ComponentParticleGenerator* GetParticleGenerator() { return dynamic_cast<ComponentParticleGenerator*>(GetComponent(COMPONENT_PARTICLE_GENERATOR)); }
		ComponentStateController* GetStateController() { return dynamic_cast<ComponentStateController*>(GetComponent(COMPONENT_STATE_CONTROLLER)); }
		ComponentPathfinder* GetPathfinder() { return dynamic_cast<ComponentPathfinder*>(GetComponent(COMPONENT_PATHFINDER)); }

		EntityManager* GetEntityManager();
		void SetEntityManager(EntityManager* manager);

		bool ContainsComponents(const ComponentTypes MASK);

		Component* RemoveGetComponent(int componentIndex);
		Component* RemoveGetComponent(ComponentTypes type);
		void RemoveComponent(Component* component);

		void AddComponent(Component* component);
		void Close();
	};
}

