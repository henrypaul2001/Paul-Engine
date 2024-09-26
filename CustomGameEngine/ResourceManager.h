#pragma once
#include <unordered_map>
#include "Model.h"
#include "TextureAtlas.h"
#include "TextFont.h"
#include "ComputeShader.h"
#include <ft2build.h>
#include "SkeletalAnimation.h"
#include "AudioFile.h"
#include FT_FREETYPE_H
namespace Engine {
	struct Cubemap {
		unsigned int id;
		std::string rootFilepath;

		~Cubemap() {
			glDeleteTextures(1, &id);
		}
	};

	struct HDREnvironment {
		unsigned int cubemapID;
		unsigned int irradianceID;
		unsigned int prefilterID;
		std::string filepath;

		~HDREnvironment() {
			glDeleteTextures(1, &cubemapID);
			glDeleteTextures(1, &irradianceID);
			glDeleteTextures(1, &prefilterID);
		}
	};

	struct Resources {
		std::unordered_map<std::string, MeshData*> meshes;
		std::unordered_map<std::string, AbstractShader*> shaders;
		std::unordered_map<std::string, Texture*> textures;
		std::unordered_map<std::string, Cubemap*> cubemaps;
		std::unordered_map<std::string, HDREnvironment*> hdrCubemaps;
		std::unordered_map<std::string, TextFont*> textFonts;
		std::unordered_map<std::string, SkeletalAnimation*> animations;
		std::unordered_map<std::string, AudioFile*> audioFiles;
		std::unordered_map<std::string, AbstractMaterial*> materials;
		std::unordered_map<std::string, AnimationSkeleton*> animationSkeletons;
	};

	enum AnisotropicFiltering;
	class RenderManager;
	class ASSIMPModelLoader;
	class ResourceManager
	{
	private:
		std::unordered_map<std::string, unsigned int> textureSlotLookup;

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
		Shader* reflectionProbeBakingPBR;

		Shader* colliderDebug;

		// Deferred shaders
		// ----------------
		Shader* deferredGeometryPass;
		Shader* deferredGeometryPassPBR;
		Shader* deferredLightingPass;
		Shader* deferredLightingPassPBR;
		Shader* deferredIBLPassPBR;
		Shader* ssrCombineShaderPBR;

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
		Shader* screenSpaceReflectionUVMappingShader;
		Shader* ssrUVMapToReflectionMap;

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

		// Creates model without adding to resource list. Instead references previously loaded MeshData in case of duplicate models. This will be the main way of loading models moving forward
		Model* CreateModel(const std::string& filepath, bool pbr, bool loadInPersistentResources = false, const unsigned int assimpPostProcess = defaultAssimpPostProcess);
		
		ComputeShader* LoadComputeShader(const char* computePath, bool loadInPersistentResources = false);
		Shader* LoadShader(std::string vertexPath, std::string fragmentPath, bool loadInPersistentResources = false);
		Shader* LoadShader(std::string vertexPath, std::string fragmentPath, std::string geometryPath, bool loadInPersistentResources = false);
		Texture* LoadTexture(std::string filepath, TextureTypes type, bool srgb, bool loadInPersistentResources = false, AnisotropicFiltering anisoFilter = (AnisotropicFiltering)-1);
		Texture* LoadTextureEmbedded(const aiTexture* embeddedTexture, const std::string& filename, TextureTypes type, bool srgb, bool loadInPersistentResources = false);
		Cubemap* LoadCubemap(const std::string& rootFilepath, const std::string& extension = ".png", bool loadInPersistentResources = false);
		HDREnvironment* LoadHDREnvironmentMap(std::string filepath, bool flipVertically = false, bool skipConversionAndBRDFLutGeneration = false, bool loadInPersistentResources = false);
		TextFont* LoadTextFont(std::string filepath, bool loadInPersistentResources = false);
		SkeletalAnimation* LoadAnimation(std::string filepath, int fileAnimationIndex = 0, bool loadInPersistentResources = false);
		AudioFile* LoadAudio(std::string filepath, float defaultVolume = 1.0f, float defaultPan = 0.0f, float defaultMinAttenuationDistance = 1.0f, float defaultMaxAttenuationDistance = FLT_MAX, bool loadInPersistentResources = false);

		bool AddMeshData(const std::string& fileNamePlusMeshName, MeshData* meshData, bool persistentResources = false) {
			std::unordered_map<std::string, MeshData*>::iterator persistentIt = this->persistentResources.meshes.find(fileNamePlusMeshName);
			std::unordered_map<std::string, MeshData*>::iterator tempIt = this->tempResources.meshes.find(fileNamePlusMeshName);

			if (persistentResources) {
				bool exists = (persistentIt != this->persistentResources.meshes.end());

				if (!exists) {
					this->persistentResources.meshes[fileNamePlusMeshName] = meshData;
				}
				else {
					std::cout << "ERROR::RESOURCEMANAGER::AddMeshData::Mesh data already exists in persistent resources" << std::endl;
					return false;
				}
			}
			else {
				bool exists = (tempIt != this->tempResources.meshes.end());

				if (!exists) {
					this->tempResources.meshes[fileNamePlusMeshName] = meshData;
				}
				else {
					std::cout << "ERROR::RESOURCEMANAGER::AddMeshData::Mesh data already exists in temp resources" << std::endl;
					return false;
				}
			}

			return true;
		}
		MeshData* GetMeshData(const std::string& fileNamePlusMeshName);
		ComputeShader* GetComputeShader(const char* computePath);
		Shader* GetShader(const std::string& vertexPath, const std::string& fragmentPath);
		Shader* GetShader(const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath);
		Texture* GetTexture(const std::string& filepath);
		Cubemap* GetCubemap(const std::string& filepath);
		HDREnvironment* GetHDREnvironmentMap(const std::string& filepath);
		TextFont* GetTextFont(const std::string& filepath);
		SkeletalAnimation* GetAnimation(const std::string& filepath, const int animationIndex = 0);
		AudioFile* GetAudio(const std::string& filepath);
		AbstractMaterial* GetMaterial(const std::string& materialName);
		AnimationSkeleton* GetAnimationSkeleton(const std::string& filename);
		bool AddMaterial(const std::string& materialName, AbstractMaterial* material, bool persistentResources = false) {
			std::unordered_map<std::string, AbstractMaterial*>::iterator persistentIt = this->persistentResources.materials.find(materialName);
			std::unordered_map<std::string, AbstractMaterial*>::iterator tempIt = this->tempResources.materials.find(materialName);

			if (persistentResources) {
				bool exists = (persistentIt != this->persistentResources.materials.end());

				if (!exists) {
					this->persistentResources.materials[materialName] = material;
				}
				else {
					std::cout << "ERROR::RESOURCEMANAGER::AddMaterial::Material already exists in persistent resources" << std::endl;
					return false;
				}
			}
			else {
				bool exists = (tempIt != this->tempResources.materials.end());

				if (!exists) {
					this->tempResources.materials[materialName] = material;
				}
				else {
					std::cout << "ERROR::RESOURCEMANAGER::AddMaterial::Material already exists in temp resources" << std::endl;
					return false;
				}
			}

			return true;
		}
		bool AddAnimationSkeleton(const std::string& filename, AnimationSkeleton* skeleton, bool persistentResources = false) {
			std::unordered_map<std::string, AnimationSkeleton*>::iterator persistentIt = this->persistentResources.animationSkeletons.find(filename);
			std::unordered_map<std::string, AnimationSkeleton*>::iterator tempIt = this->tempResources.animationSkeletons.find(filename);

			if (persistentResources) {
				bool exists = (persistentIt != this->persistentResources.animationSkeletons.end());

				if (!exists) {
					this->persistentResources.animationSkeletons[filename] = skeleton;
				}
				else {
					std::cout << "ERROR::RESOURCEMANAGER::AddAnimationSkeleton::Skeleton already exists in persistent resources" << std::endl;
					return false;
				}
			}
			else {
				bool exists = (tempIt != this->tempResources.animationSkeletons.end());

				if (!exists) {
					this->tempResources.animationSkeletons[filename] = skeleton;
				}
				else {
					std::cout << "ERROR::RESOURCEMANAGER::AddAnimationSkeleton::Skeleton already exists in temp resources" << std::endl;
					return false;
				}
			}

			return true;
		}

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
		Shader* ReflectionProbeBakingShaderPBR() const { return reflectionProbeBakingPBR; }
		Shader* ColliderDebugShader() const { return colliderDebug; }
		Shader* ScreenSpaceReflectionUVMappingShader() const { return screenSpaceReflectionUVMappingShader; }
		Shader* SSRUVMapToReflectionMap() const { return ssrUVMapToReflectionMap; }
		Shader* DeferredIBLPassPBR() const { return deferredIBLPassPBR; }
		Shader* SSRCombineShaderPBR() const { return ssrCombineShaderPBR; }
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
		
		const std::unordered_map<std::string, unsigned int>& GetTextureSlotLookupMap() const { return textureSlotLookup; }
		const unsigned int GetTextureSlot(const std::string& samplerName) const { return textureSlotLookup.at(samplerName); }

		FT_Library& GetFreeTypeLibrary() { return freetypeLib; }
	};
}