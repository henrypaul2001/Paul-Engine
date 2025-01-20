#include "SystemUIMouseInteraction.h"
#include "RenderManager.h"
namespace Engine {
	void SystemUIMouseInteraction::OnAction(const unsigned int entityID, ComponentTransform& transform, ComponentUICanvas& canvas)
	{
		if (inputManager->GetCursorLock() == false) {
			for (UIElement* ui : canvas.UIElements()) {
				if (ui->UIType() == UI_BUTTON && ui->GetActive()) {
					ProcessUIButton(dynamic_cast<UIButton*>(ui), transform);
				}
			}
		}
	}

	void SystemUIMouseInteraction::AfterAction() {}

	void SystemUIMouseInteraction::ProcessUIButton(UIButton* button, ComponentTransform& canvasTransform) const
	{
		RenderManager* renderInstance = RenderManager::GetInstance();

		glm::vec2 mousePos = inputManager->GetMousePos();

		// Invert mouse position Y axis
		const int screenHeight = renderInstance->ScreenHeight();
		mousePos.y = (float)screenHeight - mousePos.y;
		//std::cout << "inverted y = " << mousePos.y << std::endl;
		// Check if mouse position is inside of buttons boundary
		glm::vec2 buttonPos = button->Position() + glm::vec2(canvasTransform.Position().x, canvasTransform.Position().y);
		const glm::vec2& buttonScale = button->GetButtonScale() * glm::vec2(canvasTransform.Scale().x, canvasTransform.Scale().y);

		float minX = 0.0f;
		float minY = 0.0f;
		float maxX = 0.0f;
		float maxY = 0.0f;

		const ButtonTypes buttonType = button->GetButtonType();
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

		const bool mouseCollision = (mousePos.y < maxY && mousePos.y > minY && mousePos.x < maxX && mousePos.x > minX);

		// Fire mouse events to button
		if (mouseCollision) {
			// Fires once on hover
			if (!button->IsHovering()) { button->MouseEnter(); }

			if (button->IsMouseDown()) {
				// Fires once on release
				if (!inputManager->GetLeftMouseDown()) { button->MouseUp(); }
			}
			else {
				// Fires once on click
				if (inputManager->GetLeftMouseDown()) { button->MouseDown(); }
			}

			// Fires every frame during hold
			if (inputManager->GetLeftMouseDown()) { button->MouseHold(); }
		}
		else {
			// Fires once on exit
			if (button->IsHovering()) { button->MouseExit(); }
		}
	}
}