#pragma once
#include "System.h"
#include "ComponentTransform.h"
namespace Engine {
	class SystemRender : public System
	{
	public:
		SystemRender();
		~SystemRender();

		SystemTypes Name() override { return SYSTEM_RENDER; }
		void OnAction(Entity* entity) override;

	private:
		void Draw(ComponentTransform transform, ComponentGeometry geometry);
	};
}