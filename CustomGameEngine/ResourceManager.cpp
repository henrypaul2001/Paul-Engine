#include "ResourceManager.h"
namespace Engine {
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
			it->second = new Model(filepath.c_str(), pbr);
		}
		else {
			if (it->second->PBR() == pbr) {
				return it->second;
			}
			else {
				std::cout << "ERROR::RESOURCEMANAGER::LOADMODEL::Mismatch, requested model at path '" << filepath << "' already loaded with different PBR property";
				return nullptr;
			}
		}
	}

	Shader* ResourceManager::LoadShader(std::string filepath)
	{
		return nullptr;
	}

	Texture* ResourceManager::LoadTexture(std::string filepath)
	{
		return nullptr;
	}
}