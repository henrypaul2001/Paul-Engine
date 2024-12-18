#pragma once
#include "Entity.h"
#include <string>
#include "ScopeTimer.h"
namespace Engine 
{
	enum SystemTypes {
		SYSTEM_NONE = 0,
		SYSTEM_AUDIO = 1 << 0,
		SYSTEM_PHYSICS = 1 << 1,
		SYSTEM_RENDER = 1 << 2,
		SYSTEM_SHADOWMAP = 1 << 3,
		SYSTEM_COLLISION_AABB = 1 << 4,
		SYSTEM_COLLISION_SPHERE = 1 << 5,
		SYSTEM_COLLISION_SPHERE_AABB = 1 << 6,
		SYSTEM_COLLISION_BOX = 1 << 7,
		SYSTEM_COLLISION_BOX_AABB = 1 << 8,
		SYSTEM_COLLISION_BOX_SPHERE = 1 << 9,
		SYSTEM_UI_RENDER = 1 << 10,
		SYSTEM_UI_INTERACT = 1 << 11,
		SYSTEM_SKELETAL_ANIM_UPDATE = 1 << 12,
		SYSTEM_PARTICLE_UPDATE = 1 << 13,
		SYSTEM_PARTICLE_RENDER = 1 << 14,
		SYSTEM_PATHFINDING = 1 << 15,
		SYSTEM_STATE_UPDATER = 1 << 16,
		SYSTEM_REFLECTION_BAKING = 1 << 17,
		SYSTEM_FRUSTUM_CULLING = 1 << 18,
		SYSTEM_RENDER_COLLIDERS = 1 << 19,
		SYSTEM_ANIMATED_GEOMETRY_AABB = 1 << 20,
	};
	inline SystemTypes operator| (SystemTypes a, SystemTypes b) { return (SystemTypes)((int)a | (int)b); }
	inline SystemTypes operator|= (SystemTypes a, SystemTypes b) { return (SystemTypes)((int&)a |= (int)b); }

	class System
	{
	public:
		virtual void Run(const std::vector<Entity*>& entityList) {
			for (Entity* e : entityList) {
				OnAction(e);
			}
			AfterAction();
		}
		virtual void OnAction(Entity* entity) = 0;
		virtual void AfterAction() = 0;
		virtual SystemTypes Name() = 0;
	};
}