#include "SystemInstanceGeometryUpdate.h"
namespace Engine {
	SystemInstanceGeometryUpdate::SystemInstanceGeometryUpdate()
	{
	}

	SystemInstanceGeometryUpdate::~SystemInstanceGeometryUpdate()
	{
	}

	void SystemInstanceGeometryUpdate::OnAction(Entity* entity)
	{
		if ((entity->Mask() & MASK) == MASK) {
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

			if (geometry->Instanced()) {
				Update(transform, geometry);
			}
		}
	}

	void SystemInstanceGeometryUpdate::AfterAction()
	{

	}

	void SystemInstanceGeometryUpdate::Update(ComponentTransform* transform, ComponentGeometry* geometry)
	{
		const std::vector<Entity*>& sources = geometry->InstanceSources();
		geometry->ResizeInstancedTransforms();

		geometry->UpdateInstanceTransform(0, transform->GetWorldModelMatrix());
		for (int i = 0; i < sources.size(); i++) {
			geometry->UpdateInstanceTransform(i + 1, sources[i]->GetTransformComponent()->GetWorldModelMatrix());
		}

		glBindBuffer(GL_ARRAY_BUFFER, geometry->InstanceVBO());
		glBufferData(GL_ARRAY_BUFFER, sources.size() * sizeof(glm::mat4), &geometry->InstanceTransforms()[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

}