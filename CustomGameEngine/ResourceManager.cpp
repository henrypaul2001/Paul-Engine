#include "ResourceManager.h"
namespace Engine {

	ResourceManager* ResourceManager::instance = nullptr;
	ResourceManager::ResourceManager()
	{
		// Load defaults
		defaultMaterial = new Material();
		defaultMaterial->diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
		defaultMaterial->specular = glm::vec3(1.0f, 1.0f, 1.0f);
		defaultMaterial->shininess = 105.0f;
		//defaultMaterial->shininess = 60.0f;

		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		Vertex vertex;
#pragma region defaultCube
		// Back face
		vertex.Position = glm::vec3(-1.0f, -1.0f, -1.0f);
		vertex.Normal = glm::vec3(0.0f, 0.0f, -1.0f);
		vertex.TexCoords = glm::vec2(0.0f, 0.0f);
		vertices.push_back(vertex); // bottom left

		vertex.Position = glm::vec3(1.0f, 1.0f, -1.0f);
		vertex.Normal = glm::vec3(0.0f, 0.0f, -1.0f);
		vertex.TexCoords = glm::vec2(1.0f, 1.0f);
		vertices.push_back(vertex); // top right

		vertex.Position = glm::vec3(1.0f, -1.0f, -1.0f);
		vertex.Normal = glm::vec3(0.0f, 0.0f, -1.0f);
		vertex.TexCoords = glm::vec2(1.0f, 0.0f);
		vertices.push_back(vertex); // bottom right

		vertex.Position = glm::vec3(-1.0f, 1.0f, -1.0f);
		vertex.Normal = glm::vec3(0.0f, 0.0f, -1.0f);
		vertex.TexCoords = glm::vec2(0.0f, 1.0f);
		vertices.push_back(vertex); // top left

		// Front face
		vertex.Position = glm::vec3(-1.0f, -1.0f, 1.0f);
		vertex.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
		vertex.TexCoords = glm::vec2(0.0f, 0.0f);
		vertices.push_back(vertex); // bottom left

		vertex.Position = glm::vec3(1.0f, -1.0f, 1.0f);
		vertex.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
		vertex.TexCoords = glm::vec2(1.0f, 0.0f);
		vertices.push_back(vertex); // bottom right

		vertex.Position = glm::vec3(1.0f, 1.0f, 1.0f);
		vertex.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
		vertex.TexCoords = glm::vec2(1.0f, 1.0f);
		vertices.push_back(vertex); // top right

		vertex.Position = glm::vec3(-1.0f, 1.0f, 1.0f);
		vertex.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
		vertex.TexCoords = glm::vec2(0.0f, 1.0f);
		vertices.push_back(vertex); // top left

		// Left face
		vertex.Position = glm::vec3(-1.0f, 1.0f, 1.0f);
		vertex.Normal = glm::vec3(-1.0f, 0.0f, 0.0f);
		vertex.TexCoords = glm::vec2(1.0f, 0.0f);
		vertices.push_back(vertex); // top right

		vertex.Position = glm::vec3(-1.0f, 1.0f, -1.0f);
		vertex.Normal = glm::vec3(-1.0f, 0.0f, 0.0f);
		vertex.TexCoords = glm::vec2(1.0f, 1.0f);
		vertices.push_back(vertex); // top left

		vertex.Position = glm::vec3(-1.0f, -1.0f, -1.0f);
		vertex.Normal = glm::vec3(-1.0f, 0.0f, 0.0f);
		vertex.TexCoords = glm::vec2(0.0f, 1.0f);
		vertices.push_back(vertex); // bottom left

		vertex.Position = glm::vec3(-1.0f, -1.0f, 1.0f);
		vertex.Normal = glm::vec3(-1.0f, 0.0f, 0.0f);
		vertex.TexCoords = glm::vec2(0.0f, 0.0f);
		vertices.push_back(vertex); // bottom right

		// Right face
		vertex.Position = glm::vec3(1.0f, 1.0f, 1.0f);
		vertex.Normal = glm::vec3(1.0f, 0.0f, 0.0f);
		vertex.TexCoords = glm::vec2(1.0f, 0.0f);
		vertices.push_back(vertex); // top left

		vertex.Position = glm::vec3(1.0f, -1.0f, -1.0f);
		vertex.Normal = glm::vec3(1.0f, 0.0f, 0.0f);
		vertex.TexCoords = glm::vec2(0.0f, 1.0f);
		vertices.push_back(vertex); // bottom right

		vertex.Position = glm::vec3(1.0f, 1.0f, -1.0f);
		vertex.Normal = glm::vec3(1.0f, 0.0f, 0.0f);
		vertex.TexCoords = glm::vec2(1.0f, 1.0f);
		vertices.push_back(vertex); // top right

		vertex.Position = glm::vec3(1.0f, -1.0f, 1.0f);
		vertex.Normal = glm::vec3(1.0f, 0.0f, 0.0f);
		vertex.TexCoords = glm::vec2(0.0f, 0.0f);
		vertices.push_back(vertex); // bottom left

		// Bottom face
		vertex.Position = glm::vec3(-1.0f, -1.0f, -1.0f);
		vertex.Normal = glm::vec3(0.0f, -1.0f, 0.0f);
		vertex.TexCoords = glm::vec2(0.0f, 1.0f);
		vertices.push_back(vertex); // top right

		vertex.Position = glm::vec3(1.0f, -1.0f, -1.0f);
		vertex.Normal = glm::vec3(0.0f, -1.0f, 0.0f);
		vertex.TexCoords = glm::vec2(1.0f, 1.0f);
		vertices.push_back(vertex); // top left

		vertex.Position = glm::vec3(1.0f, -1.0f, 1.0f);
		vertex.Normal = glm::vec3(0.0f, -1.0f, 0.0f);
		vertex.TexCoords = glm::vec2(1.0f, 0.0f);
		vertices.push_back(vertex); // bottom left

		vertex.Position = glm::vec3(-1.0f, -1.0f, 1.0f);
		vertex.Normal = glm::vec3(0.0f, -1.0f, 0.0f);
		vertex.TexCoords = glm::vec2(0.0f, 0.0f);
		vertices.push_back(vertex); // bottom right

		// Top face
		vertex.Position = glm::vec3(-1.0f, 1.0f, -1.0f);
		vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
		vertex.TexCoords = glm::vec2(0.0f, 1.0f);
		vertices.push_back(vertex); // top left

		vertex.Position = glm::vec3(1.0f, 1.0f, 1.0f);
		vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
		vertex.TexCoords = glm::vec2(1.0f, 0.0f);
		vertices.push_back(vertex); // bottom right

		vertex.Position = glm::vec3(1.0f, 1.0f, -1.0f);
		vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
		vertex.TexCoords = glm::vec2(1.0f, 1.0f);
		vertices.push_back(vertex); // top right

		vertex.Position = glm::vec3(-1.0f, 1.0f, 1.0f);
		vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
		vertex.TexCoords = glm::vec2(0.0f, 0.0f);
		vertices.push_back(vertex); // bottom left

		indices = {
			0, 1, 2, 1, 0, 3,		// back
			4, 5, 6, 6, 7, 4,		// front
			8, 9, 10, 10, 11, 8,	// left
			12, 13, 14, 13, 12, 15, // right
			16, 17, 18, 18, 19, 16, // bottom
			20, 21, 22, 21, 20, 23  // top
		};

		GenerateBitangentTangentVectors(vertices, indices, 0);
#pragma endregion
		defaultCube = new Mesh(vertices, indices, defaultMaterial, false);
		defaultCube->SetDrawPrimitive(GL_TRIANGLES);
		vertices.clear();
		indices.clear();
		vertex = Vertex();
#pragma region defaultPlane
		vertex.Position = glm::vec3(-1.0f, 1.0f, 0.0f);
		vertex.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
		vertex.TexCoords = glm::vec2(0.0f, 1.0f);
		vertices.push_back(vertex); // top left

		vertex.Position = glm::vec3(-1.0f, -1.0f, 0.0f);
		vertex.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
		vertex.TexCoords = glm::vec2(0.0f, 0.0f);
		vertices.push_back(vertex); // bottom left

		vertex.Position = glm::vec3(1.0f, 1.0f, 0.0f);
		vertex.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
		vertex.TexCoords = glm::vec2(1.0f, 1.0f);
		vertices.push_back(vertex); // top right

		vertex.Position = glm::vec3(1.0f, -1.0f, 0.0f);
		vertex.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
		vertex.TexCoords = glm::vec2(1.0f, 0.0f);
		vertices.push_back(vertex); // bottom right

		indices = {
			0, 1, 3,
			0, 3, 2
		};

		GenerateBitangentTangentVectors(vertices, indices, 0);
#pragma endregion
		defaultPlane = new Mesh(vertices, indices, defaultMaterial, false);
		defaultPlane->SetDrawPrimitive(GL_TRIANGLES);
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

		GenerateBitangentTangentVectors(vertices, indices, 1);
#pragma endregion
		defaultSphere = new Mesh(vertices, indices, defaultMaterial, false);
		defaultSphere->SetDrawPrimitive(GL_TRIANGLE_STRIP);

		vertices.clear();
		indices.clear();
		vertex = Vertex();

		int textureOffset = 18;

		shadowMapShader = LoadShader("Shaders/depthMap.vert", "Shaders/depthMap.frag");
		cubeShadowMapShader = LoadShader("Shaders/cubeDepthMap.vert", "Shaders/cubeDepthMap.frag", "Shaders/cubeDepthMap.geom");
		defaultLitShader = LoadShader("Shaders/defaultLitNew.vert", "Shaders/defaultLitNew.frag");
		screenQuadShader = LoadShader("Shaders/screenQuad.vert", "Shaders/screenQuad.frag");
		deferredGeometryPass = LoadShader("Shaders/g_buffer.vert", "Shaders/g_buffer.frag");
		deferredLightingPass = LoadShader("Shaders/defaultDeferred.vert", "Shaders/defaultDeferred.frag");
		ssaoShader = LoadShader("Shaders/ssao.vert", "Shaders/ssao.frag");
		ssaoBlur = LoadShader("Shaders/ssao.vert", "Shaders/ssaoBlur.frag");

		screenQuadShader->Use();
		screenQuadShader->setInt("screenTexture", 0);

		defaultLitShader->Use();
		defaultLitShader->setInt("dirLight.ShadowMap", 0);
		for (int i = 0; i <= 8; i++) {
			defaultLitShader->setInt((std::string("lights[" + std::string(std::to_string(i)) + std::string("].ShadowMap"))), i + 1);
			defaultLitShader->setInt((std::string("lights[" + std::string(std::to_string(i)) + std::string("].CubeShadowMap"))), i + 8 + 1);
		}

		defaultLitShader->setInt("material.TEXTURE_DIFFUSE1", 1 + textureOffset);
		defaultLitShader->setInt("material.TEXTURE_SPECULAR1", 2 + textureOffset);
		defaultLitShader->setInt("material.TEXTURE_NORMAL1", 3 + textureOffset);
		defaultLitShader->setInt("material.TEXTURE_DISPLACE1", 4 + textureOffset);

		deferredGeometryPass->Use();
		deferredGeometryPass->setInt("material.TEXTURE_DIFFUSE1", 1 + textureOffset);
		deferredGeometryPass->setInt("material.TEXTURE_SPECULAR1", 2 + textureOffset);
		deferredGeometryPass->setInt("material.TEXTURE_NORMAL1", 3 + textureOffset);
		deferredGeometryPass->setInt("material.TEXTURE_DISPLACE1", 4 + textureOffset);

		deferredLightingPass->Use();
		deferredLightingPass->setInt("dirLight.ShadowMap", 0);
		for (int i = 0; i <= 8; i++) {
			deferredLightingPass->setInt((std::string("lights[" + std::string(std::to_string(i)) + std::string("].ShadowMap"))), i + 1);
			deferredLightingPass->setInt((std::string("lights[" + std::string(std::to_string(i)) + std::string("].CubeShadowMap"))), i + 8 + 1);
		}

		deferredLightingPass->setInt("gPosition", 18);
		deferredLightingPass->setInt("gNormal", 19);
		deferredLightingPass->setInt("gAlbedo", 20);
		deferredLightingPass->setInt("gSpecular", 21);
		deferredLightingPass->setInt("SSAO", 22);

		ssaoShader->Use();
		ssaoShader->setInt("gPosition", 0);
		ssaoShader->setInt("gNormal", 1);
		ssaoShader->setInt("texNoise", 2);
		
		ssaoBlur->Use();
		ssaoBlur->setInt("ssaoInput", 0);

		// Uniform blocks
		unsigned int defaultLitBlockLocation = glGetUniformBlockIndex(defaultLitShader->GetID(), "Common");
		unsigned int deferredGeometryPassLocation = glGetUniformBlockIndex(deferredGeometryPass->GetID(), "Common");
		unsigned int deferredLightingPassLocation = glGetUniformBlockIndex(deferredLightingPass->GetID(), "Common");
		unsigned int ssaoShaderLocation = glGetUniformBlockIndex(ssaoShader->GetID(), "Common");
		// unsigned int defaultLitPBRBlockLocation = glGetUniformBlockIndex(defaultLit_pbr.GetID(), "Matrices");
		glUniformBlockBinding(defaultLitShader->GetID(), defaultLitBlockLocation, 0);
		glUniformBlockBinding(deferredGeometryPass->GetID(), deferredGeometryPassLocation, 0);
		glUniformBlockBinding(deferredLightingPass->GetID(), deferredLightingPassLocation, 0);
		glUniformBlockBinding(ssaoShader->GetID(), ssaoShaderLocation, 0);
		// same again for pbr

		glGenBuffers(1, &uboMatrices);
		glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
		glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4) + sizeof(glm::vec3), NULL, GL_STATIC_DRAW); // resource manager
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4) + sizeof(glm::vec3));
	}

	ResourceManager::~ResourceManager()
	{
		// delete defaults
		delete defaultCube;
		delete defaultPlane;
		delete defaultSphere;
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

		// delete cubemaps
		std::unordered_map<std::string, Cubemap*>::iterator cubemapsIt = cubemaps.begin();
		while (cubemapsIt != cubemaps.end()) {
			delete cubemapsIt->second;
			cubemapsIt++;
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

	void ResourceManager::GenerateBitangentTangentVectors(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices, unsigned int offset)
	{
		// Calculate tangent and bitangent vectors
		for (unsigned int i = 0; i < indices.size() - offset; i += 3) {
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

	Shader* ResourceManager::LoadShader(std::string vertexPath, std::string fragmentPath, std::string geometryPath)
	{
		std::string combinedPath = vertexPath + "|" + fragmentPath + "|" + geometryPath;

		std::unordered_map<std::string, Shader*>::iterator it = shaders.find(combinedPath);

		if (it == shaders.end()) {
			// Shader not currently loaded
			shaders[combinedPath] = new Shader(vertexPath.c_str(), fragmentPath.c_str(), geometryPath.c_str());
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

	Cubemap* ResourceManager::LoadCubemap(std::string rootFilepath)
	{
		// First check if already loaded
		std::unordered_map<std::string, Cubemap*>::iterator it = cubemaps.find(rootFilepath);

		if (it == cubemaps.end()) {
			// Load cubemap
			unsigned int textureID;
			glGenTextures(1, &textureID);
			glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

			std::string faces[6] =
			{
				rootFilepath + "/top.png",
				rootFilepath + "/bottom.png",
				rootFilepath + "/left.png",
				rootFilepath + "/right.png",
				rootFilepath + "/front.png",
				rootFilepath + "/back.png"
			};

			int width, height, nrChannels;
			for (unsigned int i = 0; i < faces->size(); i++) {
				unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);

				if (data) {
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
				}
				else {
					std::cout << "Cubemap failed to load at path: " << faces[i] << std::endl;
				}
				stbi_image_free(data);
			}
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

			Cubemap* cubemap = new Cubemap();
			cubemap->id = textureID;
			cubemap->rootFilepath = rootFilepath;
			cubemaps[rootFilepath] = cubemap;
			return cubemaps[rootFilepath];
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