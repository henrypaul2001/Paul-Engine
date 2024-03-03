#pragma once
#include "System.h"
#include "ComponentGeometry.h"
#include "ComponentTransform.h"
#include "RenderManager.h"
namespace Engine {
	class SystemShadowMapping : public System
	{
	public:
		SystemShadowMapping();
		~SystemShadowMapping();

		SystemTypes Name() override { return SYSTEM_SHADOWMAP; }
		void OnAction(Entity* entity) override;
		void AfterAction() override;
	
		void SetDepthMapType(DepthMapType newType) { type = newType; }
	private:
		const ComponentTypes MASK = (COMPONENT_TRANSFORM | COMPONENT_GEOMETRY);
		void Draw(ComponentTransform* transform, ComponentGeometry* geometry);
		DepthMapType type;
	};
}
