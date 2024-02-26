#pragma once
namespace Engine {
	class RenderManager
	{
	public:
		RenderManager(RenderManager& other) = delete; // singleton should not be cloneable
		void operator=(const RenderManager&) = delete; // singleton should not be assignable

		void SetupShadowMapFBO(unsigned int shadowWidth, unsigned int shadowHeight);

		static RenderManager* GetInstance();
		~RenderManager();

		unsigned int* GetDepthMap() { return depthMap; }
		unsigned int* GetDepthFBO() { return depthMapFBO; }
		unsigned int ShadowWidth() { return shadowWidth; }
		unsigned int ShadowHeight() { return shadowHeight; }
	private:
		static RenderManager* instance;
		RenderManager();

		unsigned int* depthMap;
		unsigned int* depthMapFBO;
		unsigned int shadowWidth;
		unsigned int shadowHeight;
	};
}