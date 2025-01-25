#pragma once
#include "EntityManagerNew.h"
#include "ComponentGeometry.h"
#include "ComponentTransform.h"
namespace Engine {
	enum DepthMapType {
		MAP_2D,
		MAP_CUBE
	};

	class SystemShadowMapping
	{
	public:
		friend class RenderPipeline;
		SystemShadowMapping() : type(MAP_2D) {}
		~SystemShadowMapping() {}

		void OnAction(const unsigned int entityID, ComponentTransform& transform, ComponentGeometry& geometry);
	
		void SetDepthMapType(const DepthMapType newType) { type = newType; }
	private:
		DepthMapType type;
		EntityManagerNew* active_ecs;
	};
}
