#pragma once
#include "System.h"
namespace Engine {
	class SystemInstanceGeometryUpdate : public System
	{
	public:
		SystemInstanceGeometryUpdate();
		~SystemInstanceGeometryUpdate();

		SystemTypes Name() override { return SYSTEM_GEO_INSTANCE_UPDATE; }
		void OnAction(Entity* entity) override;
		void AfterAction() override;
	private:
		const ComponentTypes MASK = (COMPONENT_TRANSFORM | COMPONENT_GEOMETRY);
		void Update(ComponentTransform* transform, ComponentGeometry* geometry);
	};
}