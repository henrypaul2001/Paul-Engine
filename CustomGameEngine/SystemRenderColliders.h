#pragma once
#include "System.h"
#include "CollisionManager.h"
namespace Engine {
	class SystemRenderColliders : public System
	{
	public:
		SystemRenderColliders(CollisionManager* collisionManager);
		~SystemRenderColliders();

		SystemTypes Name() override { return SYSTEM_RENDER_COLLIDERS; }
		void Run(const std::vector<Entity*>& entityList) override;
		void OnAction(Entity* entity) override;
		void AfterAction() override;

		void DrawAABB(const glm::vec3& position, const AABBPoints& aabb, Shader* shader);
	private:
		const ComponentTypes GEOMETRY_MASK = (COMPONENT_TRANSFORM | COMPONENT_GEOMETRY);

		void DrawEntityColliders(ComponentTransform* transform, ComponentGeometry* geometry);

		unsigned int VAO, VBO;

		CollisionManager* collisionManager;
	};
}