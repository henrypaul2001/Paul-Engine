#include "SystemUIMouseInteraction.h"
#include "RenderManager.h"
namespace Engine {
	SystemUIMouseInteraction::SystemUIMouseInteraction(InputManager* inputManager)
	{
		this->inputManager = inputManager;
	}

	SystemUIMouseInteraction::~SystemUIMouseInteraction()
	{

	}

	void SystemUIMouseInteraction::OnAction(Entity* entity)
	{
		if (inputManager->GetCursorLock() == false) {
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

				if (canvas != nullptr) {
					for (UIElement* ui : canvas->UIElements()) {
						if (ui->UIType() == UI_BUTTON && ui->GetActive()) {
							ProcessUIButton(dynamic_cast<UIButton*>(ui), transform);
						}
					}
				}
			}
		}
	}

	void SystemUIMouseInteraction::AfterAction()
	{

	}

	void SystemUIMouseInteraction::ProcessUIButton(UIButton* button, ComponentTransform* canvasTransform) const
	{
		RenderManager* renderInstance = RenderManager::GetInstance();

		glm::vec2 mousePos = inputManager->GetMousePos();

		// Invert mouse position Y axis
		int screenHeight = renderInstance->ScreenHeight();
		mousePos.y = (float)screenHeight - mousePos.y;
		//std::cout << "inverted y = " << mousePos.y << std::endl;
		// Check if mouse position is inside of buttons boundary
		glm::vec2 buttonPos = button->Position() + glm::vec2(canvasTransform->Position().x, canvasTransform->Position().y);
		glm::vec2 buttonScale = button->GetButtonScale() * glm::vec2(canvasTransform->Scale().x, canvasTransform->Scale().y);

		float minX = 0.0f;
		float minY = 0.0f;
		float maxX = 0.0f;
		float maxY = 0.0f;

		ButtonTypes buttonType = button->GetButtonType();
		if (buttonType == BUTTON_IMAGE) {
			buttonPos.x = (renderInstance->ScreenWidth() / 2.0f) + ((renderInstance->ScreenWidth() / 2.0f) * buttonPos.x);
			buttonPos.y = (renderInstance->ScreenHeight() / 2.0f) + ((renderInstance->ScreenHeight() / 2.0f) * buttonPos.y);
			maxY = buttonPos.y + (buttonScale.y / 2.0f);
			minY = buttonPos.y - (buttonScale.y / 2.0f);
			minX = buttonPos.x - (buttonScale.x / 2.0f);
			maxX = buttonPos.x + (buttonScale.x / 2.0f);
		}
		else if (buttonType == BUTTON_TEXT) {
			maxY = buttonPos.y + buttonScale.y;
			maxX = buttonPos.x + buttonScale.x;
			minX = buttonPos.x;
			minY = buttonPos.y;
		}

		bool mouseCollision = false;
		if (mousePos.y < maxY && mousePos.y > minY && mousePos.x < maxX && mousePos.x > minX) {
			mouseCollision = true;
		}

		// Fire mouse events to button
		if (mouseCollision) {
			// Fire mouse enter event once
			if (!button->IsHovering()) { button->MouseEnter(); }

			if (button->IsMouseDown()) {
				if (!inputManager->GetLeftMouseDown()) { button->MouseUp(); }
			}
			else {
				if (inputManager->GetLeftMouseDown()) { button->MouseDown(); }
			}
		}
		else {
			if (button->IsHovering()) { button->MouseExit(); }
		}
	}
}