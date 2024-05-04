#pragma once
#include <unordered_map>
#include "Model.h"
#include "TextFont.h"

#include <ft2build.h>
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

		Mesh* defaultCube;
		Mesh* defaultPlane;
		Mesh* defaultSphere;

		Material* defaultMaterial;

		// Forward shaders
		// ---------------
		Shader* defaultLitPBRShader;
		Shader* defaultLitShader;
		Shader* shadowMapShader;
		Shader* cubeShadowMapShader;

		Shader* skyboxShader;

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

		FT_Library freetypeLib;

		unsigned int uiQuadVAO;
		unsigned int uiQuadVBO;
		unsigned int uiQuadEBO;
		unsigned int uboMatrices;

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

		Material* GenerateMaterial(std::vector<Texture*> diffuseMaps, std::vector<Texture*> specularMaps, std::vector<Texture*> normalMaps, std::vector<Texture*> heightMaps, float shininess, glm::vec3 diffuse, glm::vec3 specular);
	
		Mesh DefaultCube() const { return *defaultCube; }
		Mesh DefaultPlane() const { return *defaultPlane; }
		Mesh DefaultSphere() const { return *defaultSphere; }

		Material* DefaultMaterial() const { return defaultMaterial; }
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
		const unsigned int CommonUniforms() const { return uboMatrices; }
		const unsigned int GetUIQuadVAO() const { return uiQuadVAO; }
		const unsigned int GetUIQuadVBO() const { return uiQuadVBO; }
		const unsigned int GetUIQuadEBO() const { return uiQuadEBO; }
		FT_Library& GetFreeTypeLibrary() { return freetypeLib; }
	};
}