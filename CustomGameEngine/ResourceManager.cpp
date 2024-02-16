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
}