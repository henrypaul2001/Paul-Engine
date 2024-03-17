#pragma once
#include "Component.h"
namespace Engine {
	class ComponentCollision : public Component
	{
	public:
		virtual ComponentTypes ComponentType() override = 0;
		virtual void Close() override = 0;
	};
}