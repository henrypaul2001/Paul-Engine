#pragma once
#include "System.h"
#include "ReflectionProbe.h"
namespace Engine {
	class SystemReflectionBaking : public System
	{
	public:
		SystemReflectionBaking();
		~SystemReflectionBaking();

		SystemTypes Name() override { return SYSTEM_REFLECTION_BAKING; }
		void OnAction(Entity* entity) override;
		void AfterAction() override;

		void Run(const std::vector<Entity*>& entityList);

	private:
		const ComponentTypes MASK = (COMPONENT_TRANSFORM | COMPONENT_GEOMETRY);
		void Draw(ComponentTransform* transform, ComponentGeometry* geometry);

		void ConvoluteEnvironmentMap(ReflectionProbe* probe);
		void PrefilterMap(ReflectionProbe* probe);
		void BakeBRDF(ReflectionProbe* probe);
	};
}