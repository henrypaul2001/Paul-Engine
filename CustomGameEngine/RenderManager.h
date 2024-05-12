#pragma once
#include <vector>
#include <unordered_map>
#include "ResourceManager.h"
#include "RenderPipeline.h"
namespace Engine {

	enum RenderOptions {
		RENDER_SHADOWS = 1 << 0,
		RENDER_SSAO = 1 << 1,
		RENDER_BLOOM = 1 << 2,
		RENDER_TONEMAPPING = 1 << 3,
		RENDER_UI = 1 << 4,
		RENDER_IBL = 1 << 5,
		RENDER_SKYBOX = 1 << 6,
		RENDER_ENVIRONMENT_MAP = 1 << 7,
	};
	inline RenderOptions operator| (RenderOptions a, RenderOptions b) { return (RenderOptions)((int)a | (int)b); }
	inline RenderOptions operator|= (RenderOptions a, RenderOptions b) { return (RenderOptions)((int&)a |= (int)b); }
	inline RenderOptions operator& (RenderOptions a, RenderOptions b) { return (RenderOptions)((int)a & (int)b); }
	inline RenderOptions operator&= (RenderOptions a, RenderOptions b) { return (RenderOptions)((int&)a &= (int)b); }
	inline RenderOptions operator~ (RenderOptions a) { return (RenderOptions)~(int)a; }

	enum DepthMapType {
		MAP_2D,
		MAP_CUBE
	};

	struct RenderParams {
	public:
		const RenderOptions& GetRenderOptions() const { return renderOptions; }
		void SetRenderOptions(const RenderOptions& options) { renderOptions = options; }
		void EnableRenderOptions(const RenderOptions& options) { renderOptions = renderOptions | options; }
		void DisableRenderOptions(const RenderOptions& options) { renderOptions = renderOptions & ~options; }

		const float& GetExposure() const { return exposure; }
		const float& GetBloomThreshold() const { return bloomThreshold; }
		const int& GetBloomPasses() const { return bloomPasses; }
		const int& GetSSAOSamples() const { return ssaoSamples; }
		const float& GetSSAORadius() const { return ssaoRadius; }
		const float& GetSSAOBias() const { return ssaoBias; }

		void SetExposure(const float newExposure) { exposure = newExposure; }
		void SetBloomThreshold(const float newThreshold) { bloomThreshold = newThreshold; }
		void SetBloomPasses(const int newPasses) { bloomPasses = newPasses; }

		// Max sample count = 64
		void SetSSAOSamples(const int newSamples) { ssaoSamples = newSamples; }
		
		void SetSSAORadius(const float newRadius) { ssaoRadius = newRadius; }
		void SetSSAOBias(const float newBias) { ssaoBias = newBias; }
	private:
		RenderOptions renderOptions;
		
		// Tonemapping
		float exposure;

		// Bloom
		float bloomThreshold;
		int bloomPasses;

		// SSAO
		int ssaoSamples;
		float ssaoRadius;
		float ssaoBias;
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

		RenderParams* GetRenderParams() const { return renderParams; }

		unsigned int* GetDepthMap(int index, DepthMapType type);
		unsigned int* GetFlatDepthFBO() { return flatDepthMapFBO; }
		unsigned int* GetCubeDepthFBO() { return cubeDepthMapFBO; }
		unsigned int* GetTexturedFBO() { return texturedFBO; }

		unsigned int* GetGBuffer() { return gBuffer; }
		unsigned int* GPosition() { return gPosition; }
		unsigned int* GNormal() { return gNormal; }
		unsigned int* GAlbedo() { return gAlbedo; }
		unsigned int* GSpecular() { return gSpecular; }
		unsigned int* GArm() { return gArm; }
		unsigned int* GPBRFLAG() { return gPBRFLAG; }

		unsigned int* GetScreenTexture() { return screenTexture; }
		unsigned int* GetAlternateScreenTexture() { return alternateScreenTexture; }
		unsigned int* GetBloomBrightnessTexture() { return bloomBrightnessBuffer; }
		unsigned int* GetAlternateBloomBrightnessTexture() { return alternateBloomBrightnessBuffer; }

		unsigned int* GetSSAOFBO() { return ssaoFBO; }
		unsigned int* GetSSAOBlurFBO() { return ssaoBlurFBO; }
		unsigned int* SSAOColour() { return ssaoColourBuffer; }
		unsigned int* SSAOBlurColour() { return ssaoBlurColourBuffer; }
		unsigned int* SSAONoiseTexture() { return noiseTexture; }
		std::vector<glm::vec3*> SSAOKernel() { return ssaoKernel; }

		// Bloom
		unsigned int* GetBloomPingPongFBO(int index) { return pingPongFBO[index]; }
		unsigned int* GetBloomPingPongColourBuffer(int index) { return pingPongColourBuffers[index]; }

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

		void SetSkyboxTexture(Cubemap* newSkybox) { skybox = newSkybox; }
		void SetEnvironmentMap(HDREnvironment* newEnvMap) { environmentMap = newEnvMap; }
		const Cubemap* GetSkybox() const { return skybox; }
		const HDREnvironment* GetEnvironmentMap() const { return environmentMap; }

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
		void SetupEnvironmentMapFBO();

		void SetupGBuffer();

		void SetupSSAOBuffers();

		void Bind2DMap(unsigned int* map);
		void BindCubeMap(unsigned int* map);

		unsigned int* depthMap;
		std::vector<unsigned int*> flatDepthMaps;
		std::vector<unsigned int*> cubeDepthMaps; // consider using hashmap <mapIndex, texture pointer*> in future. That way, a single collection can hold both types of shadow map

		RenderPipeline* renderPipeline;

		RenderParams* renderParams;

		unsigned int* flatDepthMapFBO;
		unsigned int* cubeDepthMapFBO;

		unsigned int* screenTexture;
		unsigned int* alternateScreenTexture;

		// Forward rendering
		unsigned int* texturedFBO;

		// Deferred rendering
		unsigned int* gBuffer;
		unsigned int* gPosition, *gNormal, *gAlbedo, *gSpecular, *gArm, *gPBRFLAG;// PBRFLAG is to determine which lighting method is used in lighting pass for this pixel

		// SSAO
		unsigned int* ssaoFBO;
		unsigned int* ssaoBlurFBO;
		unsigned int* ssaoColourBuffer, *ssaoBlurColourBuffer;
		unsigned int* noiseTexture;
		std::vector<glm::vec3*> ssaoKernel;

		// Bloom
		unsigned int* bloomBrightnessBuffer;
		unsigned int* alternateBloomBrightnessBuffer;
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

		Cubemap* skybox;
		HDREnvironment* environmentMap;
	};
}