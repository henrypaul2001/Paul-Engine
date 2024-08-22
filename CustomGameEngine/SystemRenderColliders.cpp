#include "SystemRenderColliders.h"
#include "ResourceManager.h"
namespace Engine {
	SystemRenderColliders::SystemRenderColliders()
	{
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

			DrawAABB(transform, geometry);
		}
	}

	void SystemRenderColliders::AfterAction()
	{

	}

	void SystemRenderColliders::DrawAABB(ComponentTransform* transform, ComponentGeometry* geometry)
	{
		Shader* debugShader = ResourceManager::GetInstance()->ColliderDebugShader();
		debugShader->Use();

		Model* model = geometry->GetModel();

		glBindVertexArray(VAO);
		for (Mesh* mesh : model->meshes) {
			AABBPoints geometryAABB = mesh->GetGeometryAABB();

			debugShader->setFloat("MinX", geometryAABB.minX);
			debugShader->setFloat("MinY", geometryAABB.minY);
			debugShader->setFloat("MinZ", geometryAABB.minZ);

			debugShader->setFloat("MaxX", geometryAABB.maxX);
			debugShader->setFloat("MaxY", geometryAABB.maxY);
			debugShader->setFloat("MaxZ", geometryAABB.maxZ);

			glm::vec3 position = transform->GetWorldPosition();

			// Buffer single point
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3), &position);

			// Draw point using geometry shader to construct AABB
			glDrawArrays(GL_POINTS, 0, 1);
		}

		glBindVertexArray(0);
	}
}