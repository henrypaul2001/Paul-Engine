#include "Component.h"
namespace Engine
{
	Entity* Component::GetOwner()
	{
		return owner;
	}

	void Component::SetOwner(Entity* newOwner)
	{
		_ASSERT(newOwner != nullptr, "Owner cannot be null");

		owner = newOwner;
	}
}