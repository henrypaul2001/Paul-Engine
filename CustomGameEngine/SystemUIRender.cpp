#include "SystemUIRender.h"
namespace Engine {
	void SystemUIRender::OnAction(const unsigned int entityID, ComponentTransform& transform, ComponentUICanvas& canvas)
	{
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		const glm::vec3& position = transform.Position();
		const glm::vec3& scale = transform.Scale();
		const glm::vec2 canvasPosition = glm::vec2(position);
		const glm::vec2 canvasScale = glm::vec2(scale);
		for (const UIElement* uiElement : canvas.UIElements()) {
			if (uiElement->GetActive()) {
				uiElement->Draw(canvasPosition, canvasScale);
			}
		}
		glEnable(GL_DEPTH_TEST);
	}
}