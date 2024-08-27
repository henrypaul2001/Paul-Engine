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

		void DrawAABB(const glm::vec3& position, const AABBPoints& aabb, Shader* shader, const glm::vec3& colliderColour = glm::vec3(0.0f, 1.0f, 0.0f));
	private:
		const ComponentTypes GEOMETRY_MASK = (COMPONENT_TRANSFORM | COMPONENT_GEOMETRY);

		void DrawEntityColliders(ComponentTransform* transform, ComponentGeometry* geometry);
		void RenderBVHNode(const BVHNode* node);

		unsigned int VAO, VBO;

		CollisionManager* collisionManager;
	};
}