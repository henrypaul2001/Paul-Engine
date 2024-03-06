#pragma once
#include "System.h"
#include "Camera.h"
#include "ComponentGeometry.h"
#include "ComponentTransform.h"
namespace Engine {
	enum PostProcessingEffect {
		NONE = 0u,
		INVERSION = 1u,
		GRAYSCALE = 2u,
		HARD_SHARPEN = 3u,
		SUBTLE_SHARPEN = 4u,
		BLUR = 5u,
		EDGE_DETECT = 6u,
		EMBOSS = 7u,
		SOBEL = 8u,
		CUSTOM_KERNEL = 9u
	};

	class SystemRender : public System
	{
	public:
		SystemRender();
		~SystemRender();

		SystemTypes Name() override { return SYSTEM_RENDER; }
		void OnAction(Entity* entity) override;
		void AfterAction() override;

		void SetPostProcess(PostProcessingEffect effect) { postProcess = effect; }
		PostProcessingEffect GetPostProcess() { return postProcess; }

		float PostProcessKernel[9];

		//void SetCameraPointer(Camera* cam) { camera = cam; }
	private:
		//Camera* camera;
		const ComponentTypes MASK = (COMPONENT_TRANSFORM | COMPONENT_GEOMETRY);
		void Draw(ComponentTransform* transform, ComponentGeometry* geometry);
		std::vector<Shader*> shadersUsedThisFrame;

		PostProcessingEffect postProcess;
	};
}