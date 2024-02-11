#pragma once
#include "Shader.h"
#include <iostream>
#include <vector>
namespace Engine {
	enum TextureTypes {
		TEXTURE_NONE = 0,
		TEXTURE_DIFFUSE = 1 << 0,
		TEXTURE_NORMAL = 1 << 1,
		TEXTURE_METAL = 1 << 2,
		TEXTURE_DISPLACE = 1 << 3,
		TEXTURE_AO = 1 << 4,
		TEXTURE_SPECULAR = 1 << 5,
		TEXTURE_HEIGHT = 1 << 6,
		TEXTURE_ALBEDO = 1 << 7,
		TEXTURE_ROUGHNESS = 1 << 8
	};
	inline TextureTypes operator| (TextureTypes a, TextureTypes b) { return (TextureTypes)((int)a | (int)b); }
	inline TextureTypes operator|= (TextureTypes a, TextureTypes b) { return (TextureTypes)((int&)a |= (int)b); }

	struct Texture {
		unsigned int id;
		TextureTypes type;
		std::string filepath;
	};

	// #define MAX_BONE_INFLUENCE 4
	struct Vertex {
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoords;
		glm::vec3 Tangent;
		glm::vec3 Bitangent;

		// bone indices which will influence this vertex
		// int BoneIDs[MAX_BONE_INFLUENCE];
		// weights from each bone
		// float Weights[MAX_BONE_INFLUENCE];
	};

	class Mesh
	{
	public:
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<Texture> textures;
		unsigned int VAO;

		Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
		~Mesh();

		void Draw(Shader& shader);
	private:
		unsigned int VBO, EBO;
		void SetupMesh();
	};
}