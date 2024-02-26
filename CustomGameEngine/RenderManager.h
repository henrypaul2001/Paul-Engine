#pragma once
namespace Engine {
	class RenderManager
	{
	public:
		RenderManager(unsigned int shadowWidth, unsigned int shadowHeight);
		~RenderManager();
	private:
		void SetupShadowMapFBO(unsigned int shadowWidth, unsigned int shadowHeight);
		unsigned int* depthMap;
		unsigned int* depthMapFBO;
		unsigned int shadowWidth;
		unsigned int shadowHeight;
	};
}