#pragma once
#include "System.h"
#include "Camera.h"
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
		void AfterAction() override;

		//void SetCameraPointer(Camera* cam) { camera = cam; }
	private:
		//Camera* camera;
		const ComponentTypes MASK = (COMPONENT_TRANSFORM | COMPONENT_GEOMETRY);
		void Draw(ComponentTransform* transform, ComponentGeometry* geometry);
		std::vector<Shader*> shadersUsedThisFrame;
	};
}