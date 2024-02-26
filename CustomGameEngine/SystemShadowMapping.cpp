#include "SystemShadowMapping.h"
#include "ResourceManager.h"
namespace Engine {
	SystemShadowMapping::SystemShadowMapping()
	{

	}

	SystemShadowMapping::~SystemShadowMapping()
	{

	}

	void SystemShadowMapping::OnAction(Entity* entity)
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

			Draw(transform, geometry);
		}
	}

	void SystemShadowMapping::AfterAction()
	{

	}

	void SystemShadowMapping::Draw(ComponentTransform* transform, ComponentGeometry* geometry)
	{
		Shader* depthShader = ResourceManager::GetInstance()->ShadowMapShader();

		depthShader->setMat4("model", transform->GetWorldModelMatrix());

		geometry->GetModel()->Draw(*depthShader);
	}
}