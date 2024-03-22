#pragma once
#include "CollisionManager.h"
namespace Engine {
	class CollisionResponder
	{
	public:
		CollisionResponder(CollisionManager* collisonManager);
		~CollisionResponder();

		void OnAction();
		void AfterAction();
	private:
		CollisionManager* collisionManager;
	};
}