#pragma once
#include "System.h"
#include "ComponentGeometry.h"
#include "ComponentTransform.h"
namespace Engine {
	enum DepthMapType {
		MAP_2D,
		MAP_CUBE
	};

	class SystemShadowMapping : public System
	{
	public:
		SystemShadowMapping();
		~SystemShadowMapping();

		SystemTypes Name() override { return SYSTEM_SHADOWMAP; }
		void Run(const std::vector<Entity*>& entityList) override;
		void OnAction(Entity* entity) override;
		void AfterAction() override;
	
		void SetDepthMapType(DepthMapType newType) { type = newType; }
	private:
		const ComponentTypes MASK = (COMPONENT_TRANSFORM | COMPONENT_GEOMETRY);
		void Draw(ComponentTransform* transform, ComponentGeometry* geometry);
		DepthMapType type;
	};
}
