#pragma once
#include "Entity.h"
#include <string>
namespace Engine 
{
	enum SystemTypes {
		SYSTEM_NONE = 0,
		SYSTEM_AUDIO = 1 << 0,
		SYSTEM_PHYSICS = 1 << 1,
		SYSTEM_RENDER = 1 << 2,
		SYSTEM_COLLISION = 1 << 3,
		SYSTEM_SHADOWMAP = 1 << 4
	};
	inline SystemTypes operator| (SystemTypes a, SystemTypes b) { return (SystemTypes)((int)a | (int)b); }
	inline SystemTypes operator|= (SystemTypes a, SystemTypes b) { return (SystemTypes)((int&)a |= (int)b); }

	class System
	{
	public:
		virtual void OnAction(Entity* entity) = 0;
		virtual void AfterAction() = 0;
		virtual SystemTypes Name() = 0;
	};
}