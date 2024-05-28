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
namespace Engine
{
	class Entity
	{
	private:
		std::string name;
		std::vector<Component*> componentList;
		ComponentTypes mask;

	public:
		Entity(std::string name);
		~Entity();

		std::string& Name() { return name; }
		ComponentTypes& Mask() { return mask; }
		std::vector<Component*>& Components() { return componentList; }
		Component* GetComponent(ComponentTypes type);

		ComponentTransform* GetTransformComponent() { return dynamic_cast<ComponentTransform*>(GetComponent(COMPONENT_TRANSFORM)); }
		ComponentGeometry* GetGeometryComponent() { return dynamic_cast<ComponentGeometry*>(GetComponent(COMPONENT_GEOMETRY)); }
		ComponentLight* GetLightComponent() { return dynamic_cast<ComponentLight*>(GetComponent(COMPONENT_LIGHT)); }
		ComponentCollisionAABB* GetAABBCollisionComponent() { return dynamic_cast<ComponentCollisionAABB*>(GetComponent(COMPONENT_COLLISION_AABB)); }
		ComponentCollisionSphere* GetSphereCollisionComponent() { return dynamic_cast<ComponentCollisionSphere*>(GetComponent(COMPONENT_COLLISION_SPHERE)); }
		ComponentCollisionBox* GetBoxCollisionComponent() { return dynamic_cast<ComponentCollisionBox*>(GetComponent(COMPONENT_COLLISION_BOX)); }
		ComponentPhysics* GetPhysicsComponent() { return dynamic_cast<ComponentPhysics*>(GetComponent(COMPONENT_PHYSICS)); }
		ComponentUICanvas* GetUICanvasComponent() { return dynamic_cast<ComponentUICanvas*>(GetComponent(COMPONENT_UICANVAS)); }
		ComponentAnimator* GetAnimator() { return dynamic_cast<ComponentAnimator*>(GetComponent(COMPONENT_ANIMATOR)); }

		bool ContainsComponents(const ComponentTypes MASK);

		Component* RemoveGetComponent(int componentIndex);
		Component* RemoveGetComponent(ComponentTypes type);
		void RemoveComponent(Component* component);

		void AddComponent(Component* component);
		void Close();
	};
}

