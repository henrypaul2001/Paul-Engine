#pragma once
#include <unordered_map>
#include "Model.h"
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

	class ResourceManager
	{
	private:
		std::unordered_map<std::string, Model*> models;
		std::unordered_map<std::string, Shader*> shaders;
		std::unordered_map<std::string, Texture*> textures;
		std::unordered_map<std::string, Cubemap*> cubemaps;
		std::unordered_map<std::string, HDREnvironment*> hdrCubemaps;
		std::unordered_map<std::string, TextFont*> textFonts;
		std::unordered_map<std::string, SkeletalAnimation*> animations;
		std::unordered_map<std::string, AudioFile*> audioFiles;

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

		ResourceManager();
		static ResourceManager* instance;
	public:
		ResourceManager(ResourceManager& other) = delete; // singleton should not be cloneable
		void operator=(const ResourceManager&) = delete; // singleton should not be assignable

		~ResourceManager();

		static ResourceManager* GetInstance();

		Model* LoadModel(std::string filepath, bool pbr);
		Shader* LoadShader(std::string vertexPath, std::string fragmentPath);
		Shader* LoadShader(std::string vertexPath, std::string fragmentPath, std::string geometryPath);
		Texture* LoadTexture(std::string filepath, TextureTypes type, bool srgb);
		Cubemap* LoadCubemap(std::string rootFilepath);
		HDREnvironment* LoadHDREnvironmentMap(std::string filepath, bool flipVertically = false, bool skipConversionAndBRDFLutGeneration = false);
		TextFont* LoadTextFont(std::string filepath);
		SkeletalAnimation* LoadAnimation(std::string filepath, int fileAnimationIndex = 0);
		AudioFile* LoadAudio(std::string filepath, float defaultVolume = 1.0f, float defaultPan = 0.0f, float defaultMinAttenuationDistance = 1.0f, float defaultMaxAttenuationDistance = FLT_MAX);

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
		const unsigned int CommonUniforms() const { return uboMatrices; }
		const unsigned int GetUIQuadVAO() const { return uiQuadVAO; }
		const unsigned int GetUIQuadVBO() const { return uiQuadVBO; }
		
		const unsigned int GetPointVAO() const { return pointVAO; }
		const unsigned int GetPointVBO() const { return pointVBO; }

		FT_Library& GetFreeTypeLibrary() { return freetypeLib; }
	};
}