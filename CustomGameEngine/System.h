#pragma once
#include "Entity.h"
#include <string>
namespace Engine 
{
	class System
	{
	public:
		virtual void OnAction(Entity* entity) = 0;
		virtual std::string Name() = 0;
	};
}