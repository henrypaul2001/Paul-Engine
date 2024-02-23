#include "ResourceManager.h"
namespace Engine {

	ResourceManager* ResourceManager::instance = nullptr;
	ResourceManager::ResourceManager()
	{
		// Load defaults
		defaultMaterial = new Material();
		defaultMaterial->diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
		defaultMaterial->specular = glm::vec3(1.0f, 1.0f, 1.0f);
		defaultMaterial->shininess = 35.0f;

		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		Vertex vertex;
#pragma region defaultCube
		// Front face
		vertex.Position = glm::vec3(-0.5f, 0.5f, 0.5f);
		vertex.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
		vertex.TexCoords = glm::vec2(0.0f, 1.0f);
		vertices.push_back(vertex); // top left

		vertex.Position = glm::vec3(0.5f, 0.5f, 0.5f);
		vertex.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
		vertex.TexCoords = glm::vec2(1.0f, 1.0f);
		vertices.push_back(vertex); // top right

		vertex.Position = glm::vec3(0.5f, -0.5f, 0.5f);
		vertex.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
		vertex.TexCoords = glm::vec2(1.0f, 0.0f);
		vertices.push_back(vertex); // bottom right

		vertex.Position = glm::vec3(-0.5f, -0.5f, 0.5f);
		vertex.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
		vertex.TexCoords = glm::vec2(0.0f, 0.0f);
		vertices.push_back(vertex); // bottom left

		// Back face
		vertex.Position = glm::vec3(-0.5f, 0.5f, -0.5f);
		vertex.Normal = glm::vec3(0.0f, 0.0f, -1.0f);
		vertex.TexCoords = glm::vec2(1.0f, 1.0f);
		vertices.push_back(vertex); // top left

		vertex.Position = glm::vec3(0.5f, 0.5f, -0.5f);
		vertex.Normal = glm::vec3(0.0f, 0.0f, -1.0f);
		vertex.TexCoords = glm::vec2(0.0f, 1.0f);
		vertices.push_back(vertex); // top right

		vertex.Position = glm::vec3(0.5f, -0.5f, -0.5f);
		vertex.Normal = glm::vec3(0.0f, 0.0f, -1.0f);
		vertex.TexCoords = glm::vec2(0.0f, 0.0f);
		vertices.push_back(vertex); // bottom right

		vertex.Position = glm::vec3(-0.5f, -0.5f, -0.5f);
		vertex.Normal = glm::vec3(0.0f, 0.0f, -1.0f);
		vertex.TexCoords = glm::vec2(1.0f, 0.0f);
		vertices.push_back(vertex); // bottom left

		// Top face
		vertex.Position = glm::vec3(-0.5f, 0.5f, -0.5f);
		vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
		vertex.TexCoords = glm::vec2(0.0f, 1.0f);
		vertices.push_back(vertex); // top left

		vertex.Position = glm::vec3(0.5f, 0.5f, -0.5f);
		vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
		vertex.TexCoords = glm::vec2(1.0f, 1.0f);
		vertices.push_back(vertex); // top right

		vertex.Position = glm::vec3(0.5f, 0.5f, 0.5f);
		vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
		vertex.TexCoords = glm::vec2(1.0f, 0.0f);
		vertices.push_back(vertex); // bottom right

		vertex.Position = glm::vec3(-0.5f, 0.5f, 0.5f);
		vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
		vertex.TexCoords = glm::vec2(0.0f, 0.0f);
		vertices.push_back(vertex); // bottom left

		// Bottom face
		vertex.Position = glm::vec3(-0.5f, -0.5f, -0.5f);
		vertex.Normal = glm::vec3(0.0f, -1.0f, 0.0f);
		vertex.TexCoords = glm::vec2(0.0f, 0.0f);
		vertices.push_back(vertex); // top left

		vertex.Position = glm::vec3(0.5f, -0.5f, -0.5f);
		vertex.Normal = glm::vec3(0.0f, -1.0f, 0.0f);
		vertex.TexCoords = glm::vec2(1.0f, 0.0f);
		vertices.push_back(vertex); // top right

		vertex.Position = glm::vec3(0.5f, -0.5f, 0.5f);
		vertex.Normal = glm::vec3(0.0f, -1.0f, 0.0f);
		vertex.TexCoords = glm::vec2(1.0f, 1.0f);
		vertices.push_back(vertex); // bottom right

		vertex.Position = glm::vec3(-0.5f, -0.5f, 0.5f);
		vertex.Normal = glm::vec3(0.0f, -1.0f, 0.0f);
		vertex.TexCoords = glm::vec2(0.0f, 1.0f);
		vertices.push_back(vertex); // bottom left

		// Right face
		vertex.Position = glm::vec3(0.5f, 0.5f, -0.5f);
		vertex.Normal = glm::vec3(1.0f, 0.0f, 0.0f);
		vertex.TexCoords = glm::vec2(0.0f, 1.0f);
		vertices.push_back(vertex); // top left

		vertex.Position = glm::vec3(0.5f, 0.5f, 0.5f);
		vertex.Normal = glm::vec3(1.0f, 0.0f, 0.0f);
		vertex.TexCoords = glm::vec2(1.0f, 1.0f);
		vertices.push_back(vertex); // top right

		vertex.Position = glm::vec3(0.5f, -0.5f, 0.5f);
		vertex.Normal = glm::vec3(1.0f, 0.0f, 0.0f);
		vertex.TexCoords = glm::vec2(1.0f, 0.0f);
		vertices.push_back(vertex); // bottom right

		vertex.Position = glm::vec3(0.5f, -0.5f, -0.5f);
		vertex.Normal = glm::vec3(1.0f, 0.0f, 0.0f);
		vertex.TexCoords = glm::vec2(0.0f, 0.0f);
		vertices.push_back(vertex); // bottom left

		// Left face
		vertex.Position = glm::vec3(-0.5f, 0.5f, -0.5f);
		vertex.Normal = glm::vec3(-1.0f, 0.0f, 0.0f);
		vertex.TexCoords = glm::vec2(1.0f, 1.0f);
		vertices.push_back(vertex); // top left

		vertex.Position = glm::vec3(-0.5f, 0.5f, 0.5f);
		vertex.Normal = glm::vec3(-1.0f, 0.0f, 0.0f);
		vertex.TexCoords = glm::vec2(0.0f, 1.0f);
		vertices.push_back(vertex); // top right

		vertex.Position = glm::vec3(-0.5f, -0.5f, 0.5f);
		vertex.Normal = glm::vec3(-1.0f, 0.0f, 0.0f);
		vertex.TexCoords = glm::vec2(0.0f, 0.0f);
		vertices.push_back(vertex); // bottom right

		vertex.Position = glm::vec3(-0.5f, -0.5f, -0.5f);
		vertex.Normal = glm::vec3(-1.0f, 0.0f, 0.0f);
		vertex.TexCoords = glm::vec2(1.0f, 0.0f);
		vertices.push_back(vertex); // bottom left

		indices = { 
			0, 1, 2, 0, 2, 3,
			4, 5, 6, 4, 6, 7,
			8, 9, 10, 8, 10, 11,
			12, 13, 14, 12, 14, 15,
			16, 17, 18, 16, 18, 19,
			20, 21, 22, 20, 22, 23
		};

		GenerateBitangentTangentVectors(vertices, indices);
#pragma endregion
		defaultCube = new Mesh(vertices, indices, defaultMaterial, false);

		vertices.clear();
		indices.clear();
		vertex = Vertex();
#pragma region defaultPlane
		vertex.Position = glm::vec3(-0.5f, 0.5f, 0.5f);
		vertex.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
		vertex.TexCoords = glm::vec2(0.0f, 1.0f);
		// vertex.Tangent = ??
		// vertex.Bitangent = ??
		vertices.push_back(vertex); // top left

		vertex.Position = glm::vec3(0.5f, 0.5f, 0.5f);
		vertex.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
		vertex.TexCoords = glm::vec2(1.0f, 1.0f);
		// vertex.Tangent = ??
		// vertex.Bitangent = ??
		vertices.push_back(vertex); // top right

		vertex.Position = glm::vec3(0.5f, -0.5f, 0.5f);
		vertex.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
		vertex.TexCoords = glm::vec2(1.0f, 0.0f);
		// vertex.Tangent = ??
		// vertex.Bitangent = ??
		vertices.push_back(vertex); // bottom right

		vertex.Position = glm::vec3(-0.5f, -0.5f, 0.5f);
		vertex.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
		vertex.TexCoords = glm::vec2(0.0f, 0.0f);
		// vertex.Tangent = ??
		// vertex.Bitangent = ??
		vertices.push_back(vertex); // bottom left

		indices = {
			0, 1, 2, 0,
			2, 3
		};

		GenerateBitangentTangentVectors(vertices, indices);
#pragma endregion
		defaultPlane = new Mesh(vertices, indices, defaultMaterial, false);

		vertices.clear();
		indices.clear();
		vertex = Vertex();
#pragma region defaultSphere
		const unsigned int X_SEGMENTS = 64;
		const unsigned int Y_SEGMENTS = 64;
		const float PI = 3.14159265359f;

		std::vector<glm::vec3> positions;
		std::vector<glm::vec2> uv;
		std::vector<glm::vec3> normals;

		for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
		{
			for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
			{
				float xSegment = (float)x / (float)X_SEGMENTS;
				float ySegment = (float)y / (float)Y_SEGMENTS;
				float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
				float yPos = std::cos(ySegment * PI);
				float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

				positions.push_back(glm::vec3(xPos, yPos, zPos));
				uv.push_back(glm::vec2(xSegment, ySegment));
				normals.push_back(glm::vec3(xPos, yPos, zPos));
			}
		}

		bool oddRow = false;
		for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
		{
			if (!oddRow)
			{
				for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
				{
					indices.push_back(y * (X_SEGMENTS + 1) + x);
					indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
				}
			}
			else
			{
				for (int x = X_SEGMENTS; x >= 0; --x)
				{
					indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
					indices.push_back(y * (X_SEGMENTS + 1) + x);
				}
			}
			oddRow = !oddRow;
		}

		for (unsigned int i = 0; i < positions.size(); i++) {
			vertex = Vertex();
			vertex.Position = positions[i];
			vertex.Normal = normals[i];
			vertex.TexCoords = uv[i];
			vertices.push_back(vertex);
		}

		//GenerateBitangentTangentVectors(vertices, indices);
#pragma endregion
		defaultSphere = new Mesh(vertices, indices, defaultMaterial, false);
		defaultSphere->SetDrawPrimitive(GL_TRIANGLE_STRIP);

		vertices.clear();
		indices.clear();
		vertex = Vertex();
	}

	ResourceManager::~ResourceManager()
	{
		// delete defaults
		delete defaultCube;
		delete defaultMaterial;

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

	void ResourceManager::GenerateBitangentTangentVectors(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices)
	{
		// Calculate tangent and bitangent vectors
		for (unsigned int i = 0; i < indices.size(); i += 3) {
			Vertex& v0 = vertices[indices[i]];
			Vertex& v1 = vertices[indices[i + 1]];
			Vertex& v2 = vertices[indices[i + 2]];

			// Edges of triangle
			glm::vec3 deltaPos1 = v1.Position - v0.Position;
			glm::vec3 deltaPos2 = v2.Position - v0.Position;

			// UV delta
			glm::vec2 deltaUV1 = v1.TexCoords - v0.TexCoords;
			glm::vec2 deltaUV2 = v2.TexCoords - v0.TexCoords;

			// Tangent and bitangent
			float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
			glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
			glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;

			v0.Tangent += tangent;
			v1.Tangent += tangent;
			v2.Tangent += tangent;
			v0.Bitangent += bitangent;
			v1.Bitangent += bitangent;
			v2.Bitangent += bitangent;
		}

		// Normalize tangent and bitangent vectors
		for (unsigned int i = 0; i < vertices.size(); ++i) {
			vertices[i].Tangent = glm::normalize(vertices[i].Tangent);
			vertices[i].Bitangent = glm::normalize(vertices[i].Bitangent);
		}
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

	Material* ResourceManager::GenerateMaterial(std::vector<Texture*> diffuseMaps, std::vector<Texture*> specularMaps, std::vector<Texture*> normalMaps, std::vector<Texture*> heightMaps, float shininess, glm::vec3 diffuse, glm::vec3 specular)
	{
		Material* material = new Material();
		material->diffuseMaps = diffuseMaps;
		material->specularMaps = specularMaps;
		material->normalMaps = normalMaps;
		material->heightMaps = heightMaps;
		material->shininess = shininess;
		material->diffuse = diffuse;
		material->specular = specular;
		return material;
	}
}