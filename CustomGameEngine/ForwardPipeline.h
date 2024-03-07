#pragma once
#include "RenderPipeline.h"
#include "RenderManager.h"
#include "Shader.h"
#include "SystemRender.h"
#include "SystemShadowMapping.h"
namespace Engine {
	class ForwardPipeline : public RenderPipeline
	{
	public:
		ForwardPipeline();
		~ForwardPipeline();

		void Run(std::vector<System*> renderSystems, std::vector<Entity*> entities) override;
	private:
		void DirLightShadowStep();
		void ActiveLightsShadowStep();
		void SceneRenderStep();
		void ScreenTextureStep();

		RenderManager* renderInstance;

		std::vector<Entity*> entities;

		Shader* depthShader;
		Shader* cubeDepthShader;

		SystemRender* renderSystem;
		SystemShadowMapping* shadowmapSystem;

		unsigned int* depthMapFBO;
		unsigned int* cubeDepthMapFBO;

		unsigned int shadowWidth;
		unsigned int shadowHeight;

		unsigned int screenWidth;
		unsigned int screenHeight;
	};
}