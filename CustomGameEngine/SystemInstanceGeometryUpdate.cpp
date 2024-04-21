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

		for (int i = 0; i < sources.size(); i++) {
			geometry->UpdateInstanceTransform(i, sources[i]->GetTransformComponent()->GetWorldModelMatrix());
		}

		// Update on gpu
	}

}