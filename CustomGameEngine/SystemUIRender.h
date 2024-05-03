#pragma once
#include "System.h"
#include "ComponentUICanvas.h"
namespace Engine {
	class SystemUIRender : public System
	{
	public:
		SystemUIRender();
		~SystemUIRender();

		SystemTypes Name() override { return SYSTEM_UI_RENDER; }
		void OnAction(Entity* entity) override;
		void AfterAction() override;

	private:
		const ComponentTypes MASK = (COMPONENT_TRANSFORM | COMPONENT_UICANVAS);
		void Draw(ComponentTransform* transform, ComponentUICanvas* canvas);
	};
}