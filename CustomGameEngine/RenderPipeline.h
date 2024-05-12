#pragma once
#include <vector>
#include "System.h"
#include "Shader.h"
namespace Engine {
	class SystemRender;
	class SystemShadowMapping;
	class SystemUIRender;
	class RenderManager;

	enum RENDER_PIPELINE {
		FORWARD_PIPELINE,
		DEFERRED_PIPELINE
	};

	class RenderPipeline
	{
	public:
		RenderPipeline();
		~RenderPipeline();

		RENDER_PIPELINE virtual Name() = 0;
		void virtual Run(std::vector<System*> renderSystems, std::vector<Entity*> entities);
	protected:
		void virtual DirLightShadowStep();
		void virtual ActiveLightsShadowStep();
		void virtual RunShadowMapSteps();
		void virtual RunBloomStep();
		void virtual UIRenderStep();

		RenderManager* renderInstance;

		std::vector<Entity*> entities;

		Shader* depthShader;
		Shader* cubeDepthShader;

		SystemRender* renderSystem;
		SystemShadowMapping* shadowmapSystem;
		SystemUIRender* uiRenderSystem;

		unsigned int* depthMapFBO;
		unsigned int* cubeDepthMapFBO;

		unsigned int shadowWidth;
		unsigned int shadowHeight;

		unsigned int screenWidth;
		unsigned int screenHeight;

		unsigned int finalBloomTexture;
	};
}