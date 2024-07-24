#pragma once
#include <vector>
#include <unordered_map>
#include "ResourceManager.h"
#include "RenderPipeline.h"
#include "BakedData.h"
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
		RENDER_PARTICLES = 1 << 8,
		RENDER_ADVANCED_BLOOM = 1 << 9,
		RENDER_ADVANCED_BLOOM_LENS_DIRT = 1 << 10,
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

	enum AnisotropicFiltering {
		ANISO_DEFAULT = -1,
		ANISO_NONE = 0,
		ANISO_2X = 2,
		ANISO_4X = 4,
		ANISO_6X = 6,
		ANISO_8X = 8,
		ANISO_10X = 10,
		ANISO_12X = 12,
		ANISO_14X = 14,
		ANISO_16X = 16,
	};

	struct RenderParams {
	public:
		RenderParams(float exposure = 1.0f, float gamma = 1.2f, float bloomThreshold = 15.0f, int bloomPasses = 10, float advBloomThreshold = 1.0f, float advBloomSoftThreshold = 0.5f, int advBloomChainLength = 6, float advBloomFilterRadius = 0.005f, 
			float advBloomStrength = 0.04f, float advBloomDirtStrength = 5.0f, int ssaoSamples = 32, float ssaoRadius = 0.5f, float ssaoBias = 0.025f, float postProcessStrength = 1.0f, AnisotropicFiltering defaultAnisoFilter = ANISO_4X) 
			: exposure(exposure), gamma(gamma), bloomThreshold(bloomThreshold), bloomPasses(bloomPasses), advBloomThreshold(advBloomThreshold), advBloomSoftThreshold(advBloomSoftThreshold), advBloomChainLength(advBloomChainLength), advBloomFilterRadius(advBloomFilterRadius), 
			advBloomStrength(advBloomStrength), advBloomLensDirtMaskStrength(advBloomDirtStrength), ssaoSamples(ssaoSamples), ssaoRadius(ssaoRadius), ssaoBias(ssaoBias), postProcessStrength(postProcessStrength), anisotropicFiltering(defaultAnisoFilter)
		{
			SetRenderOptions(RENDER_UI | RENDER_SSAO | RENDER_SHADOWS | RENDER_ADVANCED_BLOOM | RENDER_ADVANCED_BLOOM_LENS_DIRT | RENDER_TONEMAPPING | RENDER_PARTICLES);
			EnableRenderOptions(RENDER_SKYBOX);
			DisableRenderOptions(RENDER_IBL | RENDER_ENVIRONMENT_MAP);
		}

		const RenderOptions& GetRenderOptions() const { return renderOptions; }
		void SetRenderOptions(const RenderOptions& options) { renderOptions = options; }
		void EnableRenderOptions(const RenderOptions& options) { renderOptions = renderOptions | options; }
		void DisableRenderOptions(const RenderOptions& options) { renderOptions = renderOptions & ~options; }

		const float GetPostProcessStrength() const { return postProcessStrength; }
		const float GetGamma() const { return gamma; }
		const float GetExposure() const { return exposure; }
		const float GetBloomThreshold() const { return bloomThreshold; }
		const int GetBloomPasses() const { return bloomPasses; }
		const int GetSSAOSamples() const { return ssaoSamples; }
		const float GetSSAORadius() const { return ssaoRadius; }
		const float GetSSAOBias() const { return ssaoBias; }

		// Advanced bloom
		const float GetAdvBloomThreshold() const { return advBloomThreshold; }
		const float GetAdvBloomSoftThreshold() const { return advBloomSoftThreshold; }
		const int GetAdvBloomChainLength() const { return advBloomChainLength; }
		const float GetAdvBloomFilterRadius() const { return advBloomFilterRadius; }
		const float GetAdvBloomStrength() const { return advBloomStrength; }
		const float GetAdvBloomLensDirtMaskStrength() const { return advBloomLensDirtMaskStrength; }

		void SetAdvBloomThreshold(const float newThreshold) { advBloomThreshold = newThreshold; }
		void SetAdvBloomSoftThreshold(const float newSoftThreshold) { advBloomSoftThreshold = newSoftThreshold; }
		void SetAdvBloomChainLength(const int newChainLength) { if (newChainLength < 1) { advBloomChainLength = 1; } else { advBloomChainLength = newChainLength; } }
		void SetAdvBloomFilterRadius(const float newFilterRadius) { advBloomFilterRadius = newFilterRadius; }
		void SetAdvBloomStrength(const float newStrength) { advBloomStrength = newStrength; }
		void SetAdvBloomLensDirtMaskStrength(const float newDirtStrength) { advBloomLensDirtMaskStrength = newDirtStrength; }

		void SetPostProcessStrength(const float newStrength) { postProcessStrength = newStrength; }
		void SetGamma(const float newGamma) { gamma = newGamma; }
		void SetExposure(const float newExposure) { exposure = newExposure; }
		void SetBloomThreshold(const float newThreshold) { bloomThreshold = newThreshold; }
		void SetBloomPasses(const int newPasses) { bloomPasses = newPasses; }

		// Max sample count = 64
		void SetSSAOSamples(const int newSamples) { ssaoSamples = newSamples; }
		
		void SetSSAORadius(const float newRadius) { ssaoRadius = newRadius; }
		void SetSSAOBias(const float newBias) { ssaoBias = newBias; }

		const AnisotropicFiltering GetDefaultAnisoFiltering() const { return anisotropicFiltering; }
		void SetDefaultAnisotropicFiltering(const AnisotropicFiltering newAnisoFilter) { this->anisotropicFiltering = newAnisoFilter; }
	private:
		RenderOptions renderOptions;

		// Post processing
		float postProcessStrength;

		// Tonemapping
		float exposure;
		float gamma;

		// Bloom
		float bloomThreshold;
		int bloomPasses;

		// Advanced bloom (pbr bloom)
		float advBloomThreshold;
		float advBloomSoftThreshold;
		int advBloomChainLength;
		float advBloomFilterRadius;
		float advBloomStrength;
		float advBloomLensDirtMaskStrength;

		// SSAO
		int ssaoSamples;
		float ssaoRadius;
		float ssaoBias;

		// Texture filtering
		AnisotropicFiltering anisotropicFiltering;
	};

	struct AdvBloomMip {
		glm::vec2 size;
		glm::ivec2 intSize;
		unsigned int texture;
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
		BakedData& GetBakedData() { return bakedData; }

		unsigned int* GetDepthMap(int index, DepthMapType type);
		unsigned int* GetFlatDepthFBO() const { return flatDepthMapFBO; }
		unsigned int* GetCubeDepthFBO() const { return cubeDepthMapFBO; }
		unsigned int* GetTexturedFBO() const { return texturedFBO; }
		unsigned int GetCubemapFBO() const { return cubemapFBO; }

		unsigned int* GetGBuffer() const { return gBuffer; }
		unsigned int* GPosition() const { return gPosition; }
		unsigned int* GNormal() const { return gNormal; }
		unsigned int* GAlbedo() const { return gAlbedo; }
		unsigned int* GSpecular() const { return gSpecular; }
		unsigned int* GArm() const { return gArm; }
		unsigned int* GPBRFLAG() const { return gPBRFLAG; }

		unsigned int* GetScreenTexture() const { return screenTexture; }
		unsigned int* GetAlternateScreenTexture() const { return alternateScreenTexture; }
		unsigned int* GetBloomBrightnessTexture() const { return bloomBrightnessBuffer; }
		unsigned int* GetAlternateBloomBrightnessTexture() const { return alternateBloomBrightnessBuffer; }

		unsigned int* GetSSAOFBO() const { return ssaoFBO; }
		unsigned int* GetSSAOBlurFBO() const { return ssaoBlurFBO; }
		unsigned int* SSAOColour() const { return ssaoColourBuffer; }
		unsigned int* SSAOBlurColour() const { return ssaoBlurColourBuffer; }
		unsigned int* SSAONoiseTexture() const { return noiseTexture; }
		std::vector<glm::vec3*> SSAOKernel() const { return ssaoKernel; }

		// Bloom
		unsigned int* GetBloomPingPongFBO(int index) { return pingPongFBO[index]; }
		unsigned int* GetBloomPingPongColourBuffer(int index) { return pingPongColourBuffers[index]; }

		// Env hdr map
		unsigned int* GetHDRCubeCaptureFBO() const { return hdrCubeCaptureFBO; }
		unsigned int* GetEnvironmentCubemapTexture() const { return envCubemapTexture; }

		unsigned int ShadowWidth() const { return shadowWidth; }
		unsigned int ShadowHeight() const { return shadowHeight; }
		unsigned int ScreenWidth() const { return screenWidth; }
		unsigned int ScreenHeight() const { return screenHeight; }

		unsigned int ConvertHDREquirectangularToCube(const unsigned int& textureId);
		unsigned int CreateIrradianceMap();
		void ConvoluteEnvironmentMap(const unsigned int& environmentMap, const unsigned int& irradianceMap);
		unsigned int CreatePrefilterMap(const unsigned int& environmentMap);
		unsigned int CreateBRDF();

		void SetSkyboxTexture(const std::string& skyboxFilepath) { skyboxLookup = skyboxFilepath; }
		void SetEnvironmentMap(const std::string& environmentFilepath) { environmentLookup = environmentFilepath; }
		const Cubemap* GetSkybox() const { return ResourceManager::GetInstance()->GetCubemap(skyboxLookup); }
		const HDREnvironment* GetEnvironmentMap() const { return ResourceManager::GetInstance()->GetHDREnvironmentMap(environmentLookup); }

		// Advanced bloom
		void ResizeAdvBloomMipChain(int newSize);
		const std::vector<AdvBloomMip>& GetAdvBloomMipChain() const { return advBloomMipChain; }
		void SetAdvBloomLensDirtTexture(const std::string& advBloomLensDirtMaskFilepath) { advBloomDirtMaskLookup = advBloomLensDirtMaskFilepath; }
		const Texture* GetAdvBloomLensDirtTexture() const { return ResourceManager::GetInstance()->GetTexture(advBloomDirtMaskLookup); }
		const unsigned int* GetAdvBloomFBO() const { return advBloomFBO; }

		RenderPipeline* GetRenderPipeline() const { return renderPipeline; }
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
		void SetupAdvBloom();
		void SetupCubemapFBO();

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

		unsigned int cubemapFBO;

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

		// Advanced bloom
		unsigned int* advBloomFBO;
		std::vector<AdvBloomMip> advBloomMipChain;

		// Env hdr map
		unsigned int* hdrCubeCaptureFBO;
		unsigned int* hdrCubeCaptureRBO;
		unsigned int* envCubemapTexture;

		unsigned int shadowWidth;
		unsigned int shadowHeight;

		unsigned int screenWidth;
		unsigned int screenHeight;

		std::string advBloomDirtMaskLookup;
		std::string skyboxLookup;
		std::string environmentLookup;

		BakedData bakedData;
	};
}