#pragma once
#include <vector>
#include <string>
namespace Engine
{
	enum ComponentTypes {
		COMPONENT_NONE = 0,
		COMPONENT_TRANSFORM = 1 << 0,
		COMPONENT_VELOCITY = 1 << 1,
		COMPONENT_GEOMETRY = 1 << 2
	};
	inline ComponentTypes operator| (ComponentTypes a, ComponentTypes b) { return (ComponentTypes)((int)a | (int)b); }
	inline ComponentTypes operator|= (ComponentTypes a, ComponentTypes b) { return (ComponentTypes)((int&)a |= (int)b); }

	class Entity;

	class Component
	{
	private:
		Entity* owner;
	public:
		Entity* GetOwner();
		void SetOwner(Entity* newOwner);

		virtual ComponentTypes ComponentType() = 0;
		virtual void Close() = 0;
	};
}

