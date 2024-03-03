#pragma once
#include <vector>
namespace Engine {
	enum DepthMapType {
		MAP_2D,
		MAP_CUBE
	};

	class RenderManager
	{
	public:
		RenderManager(RenderManager& other) = delete; // singleton should not be cloneable
		void operator=(const RenderManager&) = delete; // singleton should not be assignable

		void BindShadowMapTextureToFramebuffer(int mapIndex, DepthMapType type);

		static RenderManager* GetInstance(unsigned int shadowWidth, unsigned int shadowHeight);
		~RenderManager();

		unsigned int* GetDepthMap(int index, DepthMapType type);
		unsigned int* GetFlatDepthFBO() { return flatDepthMapFBO; }
		unsigned int* GetCubeDepthFBO() { return cubeDepthMapFBO; }
		unsigned int ShadowWidth() { return shadowWidth; }
		unsigned int ShadowHeight() { return shadowHeight; }
	private:
		static RenderManager* instance;
		RenderManager(unsigned int shadowWidth, unsigned int shadowHeight);

		void SetupFlatShadowMapFBO();
		void SetupCubeShadowMapFBO();
		void SetupShadowMapTextures(unsigned int shadowWidth, unsigned int shadowHeight);

		void Bind2DMap(unsigned int* map);
		void BindCubeMap(unsigned int* map);

		unsigned int* depthMap;
		std::vector<unsigned int*> flatDepthMaps;
		std::vector<unsigned int*> cubeDepthMaps;

		unsigned int* flatDepthMapFBO;
		unsigned int* cubeDepthMapFBO;
		unsigned int shadowWidth;
		unsigned int shadowHeight;
	};
}