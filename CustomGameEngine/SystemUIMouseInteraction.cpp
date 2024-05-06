#include "SystemUIMouseInteraction.h"
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
						if (ui->UIType() == UI_BUTTON) {
							ProcessUIButton(dynamic_cast<UIButton*>(ui));
						}
					}
				}
			}
		}
	}

	void SystemUIMouseInteraction::AfterAction()
	{

	}

	void SystemUIMouseInteraction::ProcessUIButton(UIButton* button) const
	{
		glm::vec2 mousePos = inputManager->GetMousePos();

		// Check if mouse position is inside of buttons boundary
		glm::vec2 buttonPos = button->Position();
		glm::vec2 buttonScale = button->Scale();
		float maxY = buttonPos.y + (buttonScale.y / 2.0f);
		float minY = buttonPos.y - (buttonScale.y / 2.0f);
		float minX = buttonPos.x - (buttonScale.x / 2.0f);
		float maxX = buttonPos.x + (buttonScale.x / 2.0f);

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