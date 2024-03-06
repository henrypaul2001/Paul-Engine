#pragma once
#include <vector>
#include <unordered_map>
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

		static RenderManager* GetInstance();
		static RenderManager* GetInstance(unsigned int shadowWidth, unsigned int shadowHeight, unsigned int screenWidth, unsigned int screenHeight);
		~RenderManager();

		unsigned int* GetDepthMap(int index, DepthMapType type);
		unsigned int* GetScreenTexture() { return screenTexture; }
		unsigned int* GetFlatDepthFBO() { return flatDepthMapFBO; }
		unsigned int* GetCubeDepthFBO() { return cubeDepthMapFBO; }
		unsigned int* GetTexturedFBO() { return texturedFBO; }
		unsigned int ShadowWidth() { return shadowWidth; }
		unsigned int ShadowHeight() { return shadowHeight; }
	private:
		static RenderManager* instance;
		RenderManager(unsigned int shadowWidth, unsigned int shadowHeight, unsigned int screenWidth, unsigned int screenHeight);

		void SetupFlatShadowMapFBO();
		void SetupCubeShadowMapFBO();
		void SetupTexturedFBO(unsigned int screenWidth, unsigned int screenHeight);
		void SetupShadowMapTextures(unsigned int shadowWidth, unsigned int shadowHeight);

		void Bind2DMap(unsigned int* map);
		void BindCubeMap(unsigned int* map);

		unsigned int* depthMap;
		std::vector<unsigned int*> flatDepthMaps;
		std::vector<unsigned int*> cubeDepthMaps; // consider using hashmap <mapIndex, texture pointer*> in future. That way, a single collection can hold both types of shadow map
		unsigned int* screenTexture;

		unsigned int* flatDepthMapFBO;
		unsigned int* cubeDepthMapFBO;
		unsigned int* texturedFBO;

		unsigned int shadowWidth;
		unsigned int shadowHeight;
	};
}