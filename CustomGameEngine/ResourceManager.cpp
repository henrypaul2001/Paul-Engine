#include "ResourceManager.h"
namespace Engine {

	ResourceManager* ResourceManager::instance = nullptr;
	ResourceManager::ResourceManager()
	{

	}

	ResourceManager::~ResourceManager()
	{
		// delete models
		std::unordered_map<std::string, Model*>::iterator modelsIt = models.begin();
		while (modelsIt != models.end()) {
			delete modelsIt->second;
			modelsIt++;
		}

		// delete shaders
		std::unordered_map<std::string, Shader*>::iterator shadersIt = shaders.begin();
		while (shadersIt != shaders.end()) {
			delete shadersIt->second;
			shadersIt++;
		}

		// delete textures
		std::unordered_map<std::string, Texture*>::iterator texturesIt = textures.begin();
		while (texturesIt != textures.end()) {
			delete texturesIt->second;
			texturesIt++;
		}

		delete instance;
	}

	ResourceManager* ResourceManager::GetInstance()
	{
		if (instance == nullptr) {
			instance = new ResourceManager();
		}
		return instance;
	}

	Model* ResourceManager::LoadModel(std::string filepath, bool pbr)
	{
		std::unordered_map<std::string, Model*>::iterator it = models.find(filepath);

		if (it == models.end()) {
			// Model not currently loaded
			models[filepath] = new Model(filepath.c_str(), pbr);
			return models[filepath];
		}
		else {
			if (it->second->PBR() == pbr) {
				return it->second;
			}
			else {
				std::cout << "ERROR::RESOURCEMANAGER::LOADMODEL:: Mismatch, requested model at path '" << filepath << "' already loaded with different PBR property" << std::endl;
				return nullptr;
			}
		}
	}

	Shader* ResourceManager::LoadShader(std::string vertexPath, std::string fragmentPath)
	{		
		std::string combinedPath = vertexPath + "|" + fragmentPath;

		std::unordered_map<std::string, Shader*>::iterator it = shaders.find(combinedPath);

		if (it == shaders.end()) {
			// Shader not currently loaded
			shaders[combinedPath] = new Shader(vertexPath.c_str(), fragmentPath.c_str());
			return shaders[combinedPath];
		}

		return it->second;
	}

	Texture* ResourceManager::LoadTexture(std::string filepath)
	{
		return nullptr;
	}
}