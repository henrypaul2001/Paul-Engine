#pragma once
#include <vector>
#include <string>
namespace Engine
{
	enum ComponentTypes {
		COMPONENT_NONE = 0,
		COMPONENT_TRANSFORM = 1 << 0,
		COMPONENT_VELOCITY = 1 << 1,
		COMPONENT_GEOMETRY = 1 << 2,
		COMPONENT_LIGHT = 1 << 3,
		COMPONENT_COLLISION_AABB = 1 << 4,
		COMPONENT_COLLISION_SPHERE = 1 << 5,
		COMPONENT_COLLISION_BOX = 1 << 6,
		COMPONENT_PHYSICS = 1 << 7,
		COMPONENT_UICANVAS = 1 << 8,
		COMPONENT_ANIMATOR = 1 << 9,
		COMPONENT_AUDIO_SOURCE = 1 << 10,
		COMPONENT_PARTICLE_GENERATOR = 1 << 11,
		COMPONENT_STATE_CONTROLLER = 1 << 12,
		COMPONENT_PATHFINDER = 1 << 13,
	};
	inline ComponentTypes operator| (ComponentTypes a, ComponentTypes b) { return (ComponentTypes)((int)a | (int)b); }
	inline ComponentTypes operator|= (ComponentTypes a, ComponentTypes b) { return (ComponentTypes)((int&)a |= (int)b); }
	inline ComponentTypes operator& (ComponentTypes a, ComponentTypes b) { return (ComponentTypes)((int)a & (int)b); }
	inline ComponentTypes operator&= (ComponentTypes a, ComponentTypes b) { return (ComponentTypes)((int&)a &= (int)b); }
	inline ComponentTypes operator~ (ComponentTypes a) { return (ComponentTypes)~(int)a; }

	class Entity;

	class Component
	{
	protected:
		Entity* owner;
	public:
		Entity* GetOwner();
		void SetOwner(Entity* newOwner);

		virtual ComponentTypes ComponentType() = 0;
		virtual void Close() = 0;

		virtual void OnAddedToEntity() {}
	};
}

