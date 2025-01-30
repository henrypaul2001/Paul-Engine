#pragma once
#include "EntityManager.h"
#include "ReflectionProbe.h"
#include "LightManager.h"
#include "Camera.h"
namespace Engine {
	class SystemReflectionBaking
	{
	public:
		SystemReflectionBaking() {}
		~SystemReflectionBaking() {}

		void OnAction(const unsigned int entityID, ComponentTransform& transform, ComponentGeometry& geometry);

		void Run(EntityManager* ecs, LightManager* lightManager, const bool discardUnfilteredCapture = true);

		void SetActiveCamera(Camera* newCamera) { activeCamera = newCamera; }
	private:
		void DrawTransparentGeometry();

		void ConvoluteEnvironmentMap(const ReflectionProbe* probe);
		void PrefilterMap(const ReflectionProbe* probe);

		Camera* activeCamera;
	
		std::map<float, std::pair<ComponentGeometry*, unsigned int>> transparentGeometry;
		
		glm::mat4 currentView;
		glm::mat4 currentProjection;
		glm::vec3 currentViewPos;

		EntityManager* ecs;
		LightManager* lightManager;
	};
}