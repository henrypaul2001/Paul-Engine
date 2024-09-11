#pragma once
#include "System.h"
#include "ReflectionProbe.h"
#include "Camera.h"
namespace Engine {
	class SystemReflectionBaking : public System
	{
	public:
		SystemReflectionBaking();
		~SystemReflectionBaking();

		SystemTypes Name() override { return SYSTEM_REFLECTION_BAKING; }
		void OnAction(Entity* entity) override;
		void AfterAction() override;

		void Run(const std::vector<Entity*>& entityList, const bool discardUnfilteredCapture = true);

		void SetActiveCamera(Camera* newCamera) { activeCamera = newCamera; }
	private:
		const ComponentTypes MASK = (COMPONENT_TRANSFORM | COMPONENT_GEOMETRY);
		void Draw(ComponentTransform* transform, ComponentGeometry* geometry);

		void DrawTransparentGeometry();

		void ConvoluteEnvironmentMap(ReflectionProbe* probe);
		void PrefilterMap(ReflectionProbe* probe);

		Camera* activeCamera;
	
		std::map<float, ComponentGeometry*> transparentGeometry;
		
		glm::mat4 currentView;
		glm::mat4 currentProjection;
		glm::vec3 currentViewPos;
	};
}