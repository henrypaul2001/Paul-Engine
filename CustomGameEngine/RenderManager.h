#pragma once
#include <vector>
#include <unordered_map>
#include "RenderPipeline.h"
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
		unsigned int* GetFlatDepthFBO() { return flatDepthMapFBO; }
		unsigned int* GetCubeDepthFBO() { return cubeDepthMapFBO; }
		unsigned int* GetTexturedFBO() { return texturedFBO; }

		unsigned int* GetGBuffer() { return gBuffer; }
		unsigned int* GPosition() { return gPosition; }
		unsigned int* GNormal() { return gNormal; }
		unsigned int* GAlbedo() { return gAlbedo; }
		unsigned int* GSpecular() { return gSpecular; }

		unsigned int* GetScreenTexture() { return screenTexture; }
		unsigned int* GetBloomBrightnessTexture() { return bloomBrightnessBuffer; }

		unsigned int* GetSSAOFBO() { return ssaoFBO; }
		unsigned int* GetSSAOBlurFBO() { return ssaoBlurFBO; }
		unsigned int* SSAOColour() { return ssaoColourBuffer; }
		unsigned int* SSAOBlurColour() { return ssaoBlurColourBuffer; }
		unsigned int* SSAONoiseTexture() { return noiseTexture; }
		std::vector<glm::vec3*> SSAOKernel() { return ssaoKernel; }

		float exposure;

		// Bloom
		unsigned int* GetBloomPingPongFBO(int index) { return pingPongFBO[index]; }
		unsigned int* GetBloomPingPongColourBuffer(int index) { return pingPongColourBuffers[index]; }
		bool bloom;
		float bloomThreshold;

		// Env hdr map
		unsigned int* GetHDRCubeCaptureFBO() { return hdrCubeCaptureFBO; }
		unsigned int* GetEnvironmentCubemapTexture() { return envCubemapTexture; }

		unsigned int ShadowWidth() { return shadowWidth; }
		unsigned int ShadowHeight() { return shadowHeight; }
		unsigned int ScreenWidth() { return screenWidth; }
		unsigned int ScreenHeight() { return screenHeight; }

		unsigned int ConvertHDREquirectangularToCube(const unsigned int& textureId);
		unsigned int CreateIrradianceMap();
		void ConvoluteEnvironmentMap(const unsigned int& environmentMap, const unsigned int& irradianceMap);
		unsigned int CreatePrefilterMap(const unsigned int& environmentMap);
		unsigned int CreateBRDF();

		RenderPipeline* GetRenderPipeline() { return renderPipeline; }
		void RunRenderPipeline(std::vector<System*> renderSystems, std::vector<Entity*> entities);
	private:
		static RenderManager* instance;
		RenderManager(unsigned int shadowWidth, unsigned int shadowHeight, unsigned int screenWidth, unsigned int screenHeight);

		void SetupFlatShadowMapFBO();
		void SetupCubeShadowMapFBO();
		void SetupTexturedFBO(unsigned int screenWidth, unsigned int screenHeight);
		void SetupShadowMapTextures(unsigned int shadowWidth, unsigned int shadowHeight);
		void SetupBloomPingPongFBO();
		void SetupEnvironmentMapFBOAndCubemap();

		void SetupGBuffer();

		void SetupSSAOBuffers();

		void Bind2DMap(unsigned int* map);
		void BindCubeMap(unsigned int* map);

		unsigned int* depthMap;
		std::vector<unsigned int*> flatDepthMaps;
		std::vector<unsigned int*> cubeDepthMaps; // consider using hashmap <mapIndex, texture pointer*> in future. That way, a single collection can hold both types of shadow map

		RenderPipeline* renderPipeline;

		unsigned int* flatDepthMapFBO;
		unsigned int* cubeDepthMapFBO;

		unsigned int* screenTexture;

		// Forward rendering
		unsigned int* texturedFBO;

		// Deferred rendering
		unsigned int* gBuffer;
		unsigned int* gPosition, *gNormal, *gAlbedo, *gSpecular;

		// SSAO
		unsigned int* ssaoFBO;
		unsigned int* ssaoBlurFBO;
		unsigned int* ssaoColourBuffer, *ssaoBlurColourBuffer;
		unsigned int* noiseTexture;
		std::vector<glm::vec3*> ssaoKernel;

		// Bloom
		unsigned int* bloomBrightnessBuffer;
		unsigned int* pingPongFBO[2];
		unsigned int* pingPongColourBuffers[2];

		// Env hdr map
		unsigned int* hdrCubeCaptureFBO;
		unsigned int* hdrCubeCaptureRBO;
		unsigned int* envCubemapTexture;

		unsigned int shadowWidth;
		unsigned int shadowHeight;

		unsigned int screenWidth;
		unsigned int screenHeight;
	};
}