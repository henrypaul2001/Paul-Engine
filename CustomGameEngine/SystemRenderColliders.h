#pragma once
#include "System.h"
namespace Engine {
	class SystemRenderColliders : public System
	{
	public:
		SystemRenderColliders();
		~SystemRenderColliders();

		SystemTypes Name() override { return SYSTEM_RENDER_COLLIDERS; }
		void Run(const std::vector<Entity*>& entityList) override;
		void OnAction(Entity* entity) override;
		void AfterAction() override;

	private:
		const ComponentTypes GEOMETRY_MASK = (COMPONENT_TRANSFORM | COMPONENT_GEOMETRY);

		void DrawAABB(ComponentTransform* transform, ComponentGeometry* geometry);

		unsigned int VAO, VBO;
	};
}