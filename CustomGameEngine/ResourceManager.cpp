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
				std::cout << "ERROR::RESOURCEMANAGER::LOADMODEL::Mismatch, requested model at path '" << filepath << "' already loaded with different PBR property" << std::endl;
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

	Texture* ResourceManager::LoadTexture(std::string filepath, TextureTypes type)
	{
		// First check if already loaded
		std::unordered_map<std::string, Texture*>::iterator it = textures.find(filepath);

		if (it == textures.end()) {
			// Load texture
			unsigned int textureID;
			glGenTextures(1, &textureID);

			int width, height, nrComponents;
			unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &nrComponents, 0);
			if (data) {
				GLenum format = GL_RGB;
				if (nrComponents == 1) {
					format = GL_RED;
				}
				else if (nrComponents == 3) {
					format = GL_RGB;
				}
				else if (nrComponents == 4) {
					format = GL_RGBA;
				}

				glBindTexture(GL_TEXTURE_2D, textureID);
				glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
				glGenerateMipmap(GL_TEXTURE_2D);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				stbi_image_free(data);
			}
			else {
				std::cout << "ERROR::RESOURCEMANAGER::TEXTURELOAD::Texture failed to load at path: " << filepath << std::endl;
				stbi_image_free(data);
			}

			Texture* texture = new Texture();
			texture->id = textureID;
			texture->type = type;
			texture->filepath = filepath;

			textures[filepath] = texture;
			return textures[filepath];
		}

		return it->second;
	}
}