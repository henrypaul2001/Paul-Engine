#pragma once
namespace Engine
{
	enum ComponentTypes {
		COMPONENT_NONE = 0,
		COMPONENT_TRANSFORM = 1 << 0,
		COMPONENT_VELOCITY = 1 << 1
	};
	inline ComponentTypes operator| (ComponentTypes a, ComponentTypes b) { return (ComponentTypes)((int)a | (int)b); }
	inline ComponentTypes operator|= (ComponentTypes a, ComponentTypes b) { return (ComponentTypes)((int&)a |= (int)b); }

	class Component
	{
	public:
		virtual ComponentTypes ComponentType() = 0;
		virtual void Close() = 0;
	};
}

