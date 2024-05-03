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

		Shader* defaultLitPBRShader;

		Shader* defaultLitShader;
		Shader* shadowMapShader;
		Shader* cubeShadowMapShader;
		Shader* screenQuadShader;
		Shader* hdrTonemappingShader;

		Shader* deferredGeometryPass;
		Shader* deferredGeometryPassPBR;
		Shader* deferredLightingPass;
		Shader* deferredLightingPassPBR;

		Shader* ssaoShader;
		Shader* ssaoBlur;

		Shader* bloomBlur;

		Shader* skyboxShader;

		FT_Library freetypeLib;

		// Environment mapping
		Shader* equirectangularToCubemapShader;
		Shader* createIrradianceShader;
		Shader* createPrefilterShader;
		Shader* createBRDFShader;

		Shader* defaultTextShader;

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
	
		Mesh DefaultCube() { return *defaultCube; }
		Mesh DefaultPlane() { return *defaultPlane; }
		Mesh DefaultSphere() { return *defaultSphere; }

		Material* DefaultMaterial() { return defaultMaterial; }
		Shader* ShadowMapShader() { return shadowMapShader; }
		Shader* CubeShadowMapShader() { return cubeShadowMapShader; }
		Shader* DefaultLitShader() { return defaultLitShader; }
		Shader* ScreenQuadShader() { return screenQuadShader; }
		Shader* HDRTonemappingShader() { return hdrTonemappingShader; }
		Shader* DeferredGeometryPass() { return deferredGeometryPass; }
		Shader* DeferredGeometryPassPBR() { return deferredGeometryPassPBR; }
		Shader* DeferredLightingPass() { return deferredLightingPass; }
		Shader* DeferredLightingPassPBR() { return deferredLightingPassPBR; }
		Shader* SSAOShader() { return ssaoShader; }
		Shader* SSABlur() { return ssaoBlur; }
		Shader* SkyboxShader() { return skyboxShader; }
		Shader* DefaultLitPBR() { return defaultLitPBRShader; }
		Shader* BloomBlurShader() { return bloomBlur; }
		Shader* EquirectangularToCubemapShader() { return equirectangularToCubemapShader; }
		Shader* CreateIrradianceShader() { return createIrradianceShader; }
		Shader* CreatePrefilterShader() { return createPrefilterShader; }
		Shader* CreateBRDFShader() { return createBRDFShader; }
		Shader* DefaultTextShader() { return defaultTextShader; }
		unsigned int CommonUniforms() { return uboMatrices; }

		FT_Library& GetFreeTypeLibrary() { return freetypeLib; }
	};
}