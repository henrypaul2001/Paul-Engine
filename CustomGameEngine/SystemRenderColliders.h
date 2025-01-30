#pragma once
#include "ComponentTransform.h"
#include "ComponentGeometry.h"
#include "CollisionManager.h"
namespace Engine {
	class SystemRenderColliders
	{
	public:
		friend class RenderPipeline;
		SystemRenderColliders();
		~SystemRenderColliders();

		void OnAction(const unsigned int entityID, ComponentTransform& transform, ComponentGeometry& geometry);
		void AfterAction();

		void DrawAABB(const glm::vec3& position, const AABBPoints& aabb, Shader* shader, const glm::vec3& colliderColour = glm::vec3(0.0f, 1.0f, 0.0f));
	private:
		void RenderBVHNode(const BVHNode* node);

		unsigned int VAO, VBO;

		CollisionManager* collisionManager;
	};
}