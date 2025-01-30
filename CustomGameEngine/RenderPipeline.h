#pragma once
#include <vector>
#include "EntityManager.h"
#include "LightManager.h"
#include "CollisionManager.h"
#include "ResourceManager.h"

#include "Shader.h"
#include "SystemRender.h"
#include "SystemShadowMapping.h"
#include "SystemUIRender.h"
#include "SystemParticleRenderer.h"
#include "SystemRenderColliders.h"
#include "SystemFrustumCulling.h"
#include "ScopeTimer.h"

namespace Engine {
	class RenderManager;
	struct AdvBloomMip;

	class RenderPipeline
	{
	public:
		RenderPipeline() : resources(ResourceManager::GetInstance()) { 
			textureLookups = &resources->GetTextureSlotLookupMap();
		}
		~RenderPipeline() {}

		SystemRender& GetRenderSystem() { return renderSystem; }
		SystemShadowMapping& GetShadowmapSystem() { return shadowmapSystem; }
		SystemUIRender& GetUIRenderSystem() { return uiRenderSystem; }
		SystemParticleRenderer& GetParticleRenderSystem() { return particleRenderSystem; }
		SystemRenderColliders& GetColliderDebugRenderSystem() { return colliderDebugRenderSystem; }

		virtual constexpr const char* PipelineName() const = 0;
		void virtual Run(EntityManager* ecs, LightManager* lightManager, CollisionManager* collisionManager, Camera* activeCamera);
	protected:
		void virtual DirLightShadowStep();
		void virtual ActiveLightsShadowStep();
		void virtual RunShadowMapSteps();
		void virtual BloomStep(const unsigned int activeBloomTexture);
		void virtual UIRenderStep();
		void virtual ForwardParticleRenderStep();
		void virtual AdvancedBloomStep(const unsigned int activeScreenTexture);
		void virtual DebugCollidersStep();

		EntityManager* ecs;
		LightManager* lightManager;
		CollisionManager* collisionManager;
		RenderManager* renderInstance;
		ResourceManager* resources;

		Shader* depthShader;
		Shader* cubeDepthShader;

		SystemRender renderSystem;
		SystemShadowMapping shadowmapSystem;
		SystemUIRender uiRenderSystem;
		SystemParticleRenderer particleRenderSystem;
		SystemRenderColliders colliderDebugRenderSystem;

		Camera* activeCamera;

		unsigned int* depthMapFBO;
		unsigned int* cubeDepthMapFBO;

		unsigned int shadowWidth;
		unsigned int shadowHeight;

		unsigned int screenWidth;
		unsigned int screenHeight;

		unsigned int finalBloomTexture;
		unsigned int advBloomSourceTexture;

		const std::unordered_map<std::string, unsigned int>* textureLookups;

	private:
		void AdvBloomDownsampleStep(const std::vector<AdvBloomMip>& mipChain, const float threshold, const float softThreshold);
		void AdvBloomUpsampleStep(const std::vector<AdvBloomMip>& mipChain, const float filterRadius);
		void AdvBloomCombineStep(const bool renderDirtMask, const float bloomStrength, const float lensDirtStrength);
	};
}