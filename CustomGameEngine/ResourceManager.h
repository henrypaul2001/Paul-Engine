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

		ResourceManager();
		static ResourceManager* instance;
	public:
		ResourceManager(ResourceManager& other) = delete; // singleton should not be cloneable
		void operator=(const ResourceManager&) = delete; // singleton should not be assignable

		~ResourceManager();

		static ResourceManager* GetInstance();

		Model* LoadModel(std::string filepath, bool pbr);
		Shader* LoadShader(std::string vertexPath, std::string fragmentPath);
		Texture* LoadTexture(std::string filepath, TextureTypes type);

		Material* GenerateMaterial(std::vector<Texture*> diffuseMaps, std::vector<Texture*> specularMaps, std::vector<Texture*> normalMaps, std::vector<Texture*> heightMaps, float shininess, glm::vec3 diffuse, glm::vec3 specular);
	};
}