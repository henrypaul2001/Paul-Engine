#pragma once
#include <vector>
#include "System.h"
#include "Shader.h"
#include "SystemParticleRenderer.h"
namespace Engine {
	class SystemRender;
	class SystemShadowMapping;
	class SystemUIRender;
	class RenderManager;
	struct AdvBloomMip;

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
		void virtual BloomStep();
		void virtual UIRenderStep();
		void virtual ForwardParticleRenderStep();
		void virtual AdvancedBloomStep();
		void virtual AdvancedBloomCombineStep();

		RenderManager* renderInstance;

		std::vector<Entity*> entities;

		Shader* depthShader;
		Shader* cubeDepthShader;

		SystemRender* renderSystem;
		SystemShadowMapping* shadowmapSystem;
		SystemUIRender* uiRenderSystem;
		SystemParticleRenderer* particleRenderSystem;

		std::vector<System*> renderSystems;

		unsigned int* depthMapFBO;
		unsigned int* cubeDepthMapFBO;

		unsigned int shadowWidth;
		unsigned int shadowHeight;

		unsigned int screenWidth;
		unsigned int screenHeight;

		unsigned int finalBloomTexture;

	private:
		void AdvBloomDownsampleStep(const std::vector<AdvBloomMip>& mipChain, const float threshold, const float softThreshold);
		void AdvBloomUpsampleStep(const std::vector<AdvBloomMip>& mipChain, const float filterRadius);
	};
}