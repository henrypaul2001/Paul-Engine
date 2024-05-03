#include "SystemUIRender.h"
namespace Engine {
	SystemUIRender::SystemUIRender()
	{

	}

	SystemUIRender::~SystemUIRender()
	{

	}

	void SystemUIRender::OnAction(Entity* entity)
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

			ComponentUICanvas* canvas = nullptr;
			for (Component* c : components) {
				canvas = dynamic_cast<ComponentUICanvas*>(c);
				if (canvas != nullptr) {
					break;
				}
			}

			Draw(transform, canvas);
		}
	}

	void SystemUIRender::AfterAction()
	{

	}

	void SystemUIRender::Draw(ComponentTransform* transform, ComponentUICanvas* canvas)
	{
		for (UIElement* uiElement : canvas->UIElements()) {
			uiElement->Draw();
		}
	}
}