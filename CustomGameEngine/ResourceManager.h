#pragma once
#include <unordered_map>
#include "Model.h"
#include "TextureAtlas.h"
#include "TextFont.h"

#include <ft2build.h>
#include "SkeletalAnimation.h"
#include "AudioFile.h"
#include FT_FREETYPE_H
namespace Engine {
	struct Cubemap {
		unsigned int id;
		std::string rootFilepath;
	};

	struct HDREnvironment {
		unsigned int cubemapID;
		unsigned int irradianceID;
		unsigned int prefilterID;
		unsigned int brdf_lutID;
		std::string filepath;
	};

	struct Resources {
		std::unordered_map<std::string, Model*> models;
		std::unordered_map<std::string, Shader*> shaders;
		std::unordered_map<std::string, Texture*> textures;
		std::unordered_map<std::string, Cubemap*> cubemaps;
		std::unordered_map<std::string, HDREnvironment*> hdrCubemaps;
		std::unordered_map<std::string, TextFont*> textFonts;
		std::unordered_map<std::string, SkeletalAnimation*> animations;
		std::unordered_map<std::string, AudioFile*> audioFiles;
	};

	enum AnisotropicFiltering;
	class RenderManager;
	class ResourceManager
	{
	private:
		std::unordered_map<TextureTypes, aiTextureType> modelLoaderTextureTranslations;

		Resources persistentResources;
		Resources tempResources;

		Mesh* defaultCube;
		Mesh* defaultPlane;
		Mesh* defaultSphere;

		Material* defaultMaterial;
		PBRMaterial* defaultMaterialPBR;

		// Forward shaders
		// ---------------
		Shader* defaultLitPBRShader;
		Shader* defaultLitShader;
		Shader* shadowMapShader;
		Shader* cubeShadowMapShader;

		Shader* skyboxShader;
		Shader* particleShader;
		Shader* pointParticleShader;

		Shader* reflectionProbeBaking;

		// Deferred shaders
		// ----------------
		Shader* deferredGeometryPass;
		Shader* deferredGeometryPassPBR;
		Shader* deferredLightingPass;
		Shader* deferredLightingPassPBR;

		// Post processing / graphics effects shaders
		// ------------------------------------------
		Shader* ssaoShader;
		Shader* ssaoBlur;
		Shader* bloomBlur;
		Shader* screenQuadShader;
		Shader* hdrTonemappingShader;
		Shader* advBloomDownsampleShader;
		Shader* advBloomUpsampleShader;
		Shader* advBloomCombineShader;

		// Environment mapping
		// -------------------
		Shader* equirectangularToCubemapShader;
		Shader* createIrradianceShader;
		Shader* createPrefilterShader;
		Shader* createBRDFShader;

		// UI shaders
		// ----------
		Shader* defaultTextShader;
		Shader* defaultImageShader;
		Shader* uiBackgroundShader;

		FT_Library freetypeLib;

		unsigned int uiQuadVAO;
		unsigned int uiQuadVBO;
		unsigned int uboMatrices;

		unsigned int pointVAO;
		unsigned int pointVBO;

		void GenerateBitangentTangentVectors(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices, unsigned int offset);

		void ClearResources(Resources& resources);

		ResourceManager();
		static ResourceManager* instance;
	public:
		ResourceManager(ResourceManager& other) = delete; // singleton should not be cloneable
		void operator=(const ResourceManager&) = delete; // singleton should not be assignable

		~ResourceManager();

		static ResourceManager* GetInstance();

		Model* LoadModel(std::string filepath, bool pbr, bool loadInPersistentResources = false, const unsigned int assimpPostProcess = defaultAssimpPostProcess);
		Shader* LoadShader(std::string vertexPath, std::string fragmentPath, bool loadInPersistentResources = false);
		Shader* LoadShader(std::string vertexPath, std::string fragmentPath, std::string geometryPath, bool loadInPersistentResources = false);
		Texture* LoadTexture(std::string filepath, TextureTypes type, bool srgb, bool loadInPersistentResources = false, AnisotropicFiltering anisoFilter = (AnisotropicFiltering)-1);
		Texture* LoadTextureEmbedded(const aiTexture* embeddedTexture, const std::string& filename, TextureTypes type, bool srgb, bool loadInPersistentResources = false);
		Cubemap* LoadCubemap(std::string rootFilepath, bool loadInPersistentResources = false);
		HDREnvironment* LoadHDREnvironmentMap(std::string filepath, bool flipVertically = false, bool skipConversionAndBRDFLutGeneration = false, bool loadInPersistentResources = false);
		TextFont* LoadTextFont(std::string filepath, bool loadInPersistentResources = false);
		SkeletalAnimation* LoadAnimation(std::string filepath, int fileAnimationIndex = 0, bool loadInPersistentResources = false);
		AudioFile* LoadAudio(std::string filepath, float defaultVolume = 1.0f, float defaultPan = 0.0f, float defaultMinAttenuationDistance = 1.0f, float defaultMaxAttenuationDistance = FLT_MAX, bool loadInPersistentResources = false);

		Model* GetModel(const std::string& filepath);
		Shader* GetShader(const std::string& vertexPath, const std::string& fragmentPath);
		Shader* GetShader(const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath);
		Texture* GetTexture(const std::string& filepath);
		Cubemap* GetCubemap(const std::string& filepath);
		HDREnvironment* GetHDREnvironmentMap(const std::string& filepath);
		TextFont* GetTextFont(const std::string& filepath);
		SkeletalAnimation* GetAnimation(const std::string& filepath, const int animationIndex = 0);
		AudioFile* GetAudio(const std::string& filepath);

		Material* GenerateMaterial(std::vector<Texture*> diffuseMaps, std::vector<Texture*> specularMaps, std::vector<Texture*> normalMaps, std::vector<Texture*> heightMaps, float shininess, glm::vec3 diffuse, glm::vec3 specular);
	
		Mesh DefaultCube() const { return *defaultCube; }
		Mesh DefaultPlane() const { return *defaultPlane; }
		Mesh DefaultSphere() const { return *defaultSphere; }

		Material* DefaultMaterial() const { return defaultMaterial; }
		PBRMaterial* DefaultMaterialPBR() const { return defaultMaterialPBR; }
		Shader* ShadowMapShader() const { return shadowMapShader; }
		Shader* CubeShadowMapShader() const { return cubeShadowMapShader; }
		Shader* DefaultLitShader() const { return defaultLitShader; }
		Shader* ScreenQuadShader() const { return screenQuadShader; }
		Shader* HDRTonemappingShader() const { return hdrTonemappingShader; }
		Shader* DeferredGeometryPass() const { return deferredGeometryPass; }
		Shader* DeferredGeometryPassPBR() const { return deferredGeometryPassPBR; }
		Shader* DeferredLightingPass() const { return deferredLightingPass; }
		Shader* DeferredLightingPassPBR() const { return deferredLightingPassPBR; }
		Shader* SSAOShader() const { return ssaoShader; }
		Shader* SSABlur() const { return ssaoBlur; }
		Shader* SkyboxShader() const { return skyboxShader; }
		Shader* DefaultLitPBR() const { return defaultLitPBRShader; }
		Shader* BloomBlurShader() const { return bloomBlur; }
		Shader* EquirectangularToCubemapShader() const { return equirectangularToCubemapShader; }
		Shader* CreateIrradianceShader() const { return createIrradianceShader; }
		Shader* CreatePrefilterShader() const { return createPrefilterShader; }
		Shader* CreateBRDFShader() const { return createBRDFShader; }
		Shader* DefaultTextShader() const { return defaultTextShader; }
		Shader* DefaultImageShader() const { return defaultImageShader; }
		Shader* DefaultUIBackgroundShader() const { return uiBackgroundShader; }
		Shader* DefaultParticleShader() const { return particleShader; }
		Shader* DefaultPointParticleShader() const { return pointParticleShader; }
		Shader* AdvBloomDownsampleShader() const { return advBloomDownsampleShader; }
		Shader* AdvBloomUpsampleShader() const { return advBloomUpsampleShader; }
		Shader* AdvBloomCombineShader() const { return advBloomCombineShader; }
		Shader* ReflectionProbeBakingShader() const { return reflectionProbeBaking; }
		const unsigned int CommonUniforms() const { return uboMatrices; }
		const unsigned int GetUIQuadVAO() const { return uiQuadVAO; }
		const unsigned int GetUIQuadVBO() const { return uiQuadVBO; }
		
		const unsigned int GetPointVAO() const { return pointVAO; }
		const unsigned int GetPointVBO() const { return pointVBO; }

		void ClearTempResources() { ClearResources(tempResources); }
		void ClearPersistentResources() { ClearResources(persistentResources); }

		void ResetModelLoaderTextureTranslationsToDefault() {
			// non pbr
			modelLoaderTextureTranslations[TEXTURE_DIFFUSE] = aiTextureType_DIFFUSE;
			modelLoaderTextureTranslations[TEXTURE_SPECULAR] = aiTextureType_SPECULAR;
			modelLoaderTextureTranslations[TEXTURE_NORMAL] = aiTextureType_HEIGHT;
			modelLoaderTextureTranslations[TEXTURE_HEIGHT] = aiTextureType_AMBIENT;
			modelLoaderTextureTranslations[TEXTURE_OPACITY] = aiTextureType_OPACITY;

			// pbr
			modelLoaderTextureTranslations[TEXTURE_ALBEDO] = aiTextureType_DIFFUSE;
			modelLoaderTextureTranslations[TEXTURE_METALLIC] = aiTextureType_SHININESS;
			modelLoaderTextureTranslations[TEXTURE_ROUGHNESS] = aiTextureType_SPECULAR;
			modelLoaderTextureTranslations[TEXTURE_AO] = aiTextureType_AMBIENT;
		}
		void SetModelLoaderTextureTranslation(const TextureTypes target, const aiTextureType translation) { modelLoaderTextureTranslations[target] = translation; }
		aiTextureType GetTranslatedTexture(const TextureTypes target) { return modelLoaderTextureTranslations[target]; }
		const std::unordered_map<TextureTypes, aiTextureType>& GetTextureTranslations() { return modelLoaderTextureTranslations; }

		FT_Library& GetFreeTypeLibrary() { return freetypeLib; }
	};
}