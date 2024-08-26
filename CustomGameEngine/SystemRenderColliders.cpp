#include "SystemRenderColliders.h"
#include "ResourceManager.h"
namespace Engine {
	SystemRenderColliders::SystemRenderColliders(CollisionManager* collisionManager)
	{
		this->collisionManager = collisionManager;

		VAO = 0;
		VBO = 0;
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);

		// vertex position
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
	}

	SystemRenderColliders::~SystemRenderColliders()
	{

	}

	void SystemRenderColliders::Run(const std::vector<Entity*>& entityList)
	{
		System::Run(entityList);

		// Render BVH Tree
		const BVHTree* bvhTree = collisionManager->GetBVHTree();


	}

	void SystemRenderColliders::OnAction(Entity* entity)
	{
		if ((entity->Mask() & GEOMETRY_MASK) == GEOMETRY_MASK) {
			std::vector<Component*> components = entity->Components();

			ComponentTransform* transform = nullptr;
			for (Component* c : components) {
				transform = dynamic_cast<ComponentTransform*>(c);
				if (transform != nullptr) {
					break;
				}
			}

			ComponentGeometry* geometry = nullptr;
			for (Component* c : components) {
				geometry = dynamic_cast<ComponentGeometry*>(c);
				if (geometry != nullptr) {
					break;
				}
			}

			DrawEntityColliders(transform, geometry);
		}
	}

	void SystemRenderColliders::AfterAction()
	{

	}

	void SystemRenderColliders::DrawAABB(const glm::vec3& position, const AABBPoints& aabb, Shader* shader)
	{
		shader->Use();

		glBindVertexArray(VAO);

		shader->setFloat("MinX", aabb.minX);
		shader->setFloat("MinY", aabb.minY);
		shader->setFloat("MinZ", aabb.minZ);

		shader->setFloat("MaxX", aabb.maxX);
		shader->setFloat("MaxY", aabb.maxY);
		shader->setFloat("MaxZ", aabb.maxZ);

		// Buffer single point
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3), &position);

		// Draw point using geometry shader to construct AABB
		glDrawArrays(GL_POINTS, 0, 1);

		glBindVertexArray(0);
	}

	void SystemRenderColliders::DrawEntityColliders(ComponentTransform* transform, ComponentGeometry* geometry)
	{
		Shader* debugShader = ResourceManager::GetInstance()->ColliderDebugShader();
		Model* model = geometry->GetModel();

		for (Mesh* mesh : model->meshes) {
			AABBPoints geometryAABB = mesh->GetGeometryAABB();
			glm::vec3 position = transform->GetWorldPosition();

			DrawAABB(position, geometryAABB, debugShader);
		}
	}
}