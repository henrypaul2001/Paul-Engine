#pragma once
#include <unordered_map>
#include "Model.h"
namespace Engine {
	class ResourceManager
	{
	private:
		std::unordered_map<std::string, Model*> models;
		std::unordered_map<std::string, Shader*> shaders;
		std::unordered_map<std::string, Texture*> textures;

		Mesh* defaultCube;
		Mesh* defaultPlane;
		Mesh* defaultSphere;

		Material* defaultMaterial;

		Shader* defaultLitShader;
		Shader* shadowMapShader;
		Shader* cubeShadowMapShader;
		Shader* screenQuadShader;

		Shader* deferredGeometryPass;
		Shader* deferredLightingPass;

		Shader* ssaoShader;
		Shader* ssaoBlur;

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
		Texture* LoadTexture(std::string filepath, TextureTypes type);

		Material* GenerateMaterial(std::vector<Texture*> diffuseMaps, std::vector<Texture*> specularMaps, std::vector<Texture*> normalMaps, std::vector<Texture*> heightMaps, float shininess, glm::vec3 diffuse, glm::vec3 specular);
	
		Mesh* DefaultCube() { return defaultCube; }
		Mesh* DefaultPlane() { return defaultPlane; }
		Mesh* DefaultSphere() { return defaultSphere; }
		Material* DefaultMaterial() { return defaultMaterial; }
		Shader* ShadowMapShader() { return shadowMapShader; }
		Shader* CubeShadowMapShader() { return cubeShadowMapShader; }
		Shader* DefaultLitShader() { return defaultLitShader; }
		Shader* ScreenQuadShader() { return screenQuadShader; }
		Shader* DeferredGeometryPass() { return deferredGeometryPass; }
		Shader* DeferredLightingPass() { return deferredLightingPass; }
		Shader* SSAOShader() { return ssaoShader; }
		Shader* SSABlur() { return ssaoBlur; }
		unsigned int CommonUniforms() { return uboMatrices; }
	};
}