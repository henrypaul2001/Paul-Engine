#pragma once
#include <vector>
namespace Engine {
	class RenderManager
	{
	public:
		RenderManager(RenderManager& other) = delete; // singleton should not be cloneable
		void operator=(const RenderManager&) = delete; // singleton should not be assignable

		void SetupShadowMapFBO();
		void SetupShadowMapTextures(unsigned int shadowWidth, unsigned int shadowHeight);
		void BindShadowMapTextureToFramebuffer(int mapIndex);

		static RenderManager* GetInstance(unsigned int shadowWidth, unsigned int shadowHeight);
		~RenderManager();

		unsigned int* GetDepthMap() { return depthMap; }
		unsigned int* GetDepthFBO() { return depthMapFBO; }
		unsigned int ShadowWidth() { return shadowWidth; }
		unsigned int ShadowHeight() { return shadowHeight; }
	private:
		static RenderManager* instance;
		RenderManager(unsigned int shadowWidth, unsigned int shadowHeight);

		unsigned int* depthMap;
		std::vector<unsigned int*> activeDepthMaps;

		unsigned int* depthMapFBO;
		unsigned int shadowWidth;
		unsigned int shadowHeight;
	};
}