#pragma once
#include "System.h"
#include "ComponentGeometry.h"
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
		const ComponentTypes MASK = (COMPONENT_TRANSFORM | COMPONENT_GEOMETRY);
		void Draw(ComponentTransform* transform, ComponentGeometry* geometry);
	};
}