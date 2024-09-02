#include "SystemUIRender.h"
namespace Engine {
	SystemUIRender::SystemUIRender()
	{

	}

	SystemUIRender::~SystemUIRender()
	{

	}

	void SystemUIRender::Run(const std::vector<Entity*>& entityList)
	{
		SCOPE_TIMER("SystemUIRender::Run");
		System::Run(entityList);
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
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glm::vec2 canvasPosition = glm::vec2(transform->Position().x, transform->Position().y);
		glm::vec2 canvasScale = glm::vec2(transform->Scale().x, transform->Scale().y);
		for (UIElement* uiElement : canvas->UIElements()) {
			if (uiElement->GetActive()) {
				uiElement->Draw(canvasPosition, canvasScale);
			}
		}
		glEnable(GL_DEPTH_TEST);
	}
}