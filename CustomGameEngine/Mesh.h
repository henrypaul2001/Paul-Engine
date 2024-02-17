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

	static std::string ConvertTextureTypeToString(TextureTypes type) {
		switch (type) {
		case TEXTURE_NONE:
			return "TEXTURE_NONE";
		case TEXTURE_DIFFUSE:
			return "TEXTURE_DIFFUSE";
		case TEXTURE_NORMAL:
			return "TEXTURE_NORMAL";
		case TEXTURE_METAL:
			return "TEXTURE_METAL";
		case TEXTURE_DISPLACE:
			return "TEXTURE_DISPLACE";
		case TEXTURE_AO:
			return "TEXTURE_AO";
		case TEXTURE_SPECULAR:
			return "TEXTURE_SPECULAR";
		case TEXTURE_HEIGHT:
			return "TEXTURE_HEIGHT";
		case TEXTURE_ALBEDO:
			return "TEXTURE_ALBEDO";
		case TEXTURE_ROUGHNESS:
			return "TEXTURE_ROUGHNESS";
		}
		return "NULL";
	}

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
		std::vector<Texture*> textures;
		unsigned int VAO;

		Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture*> textures, bool pbr);
		~Mesh();

		bool PBR() { return pbr; }

		void Draw(Shader& shader);
	private:
		unsigned int VBO, EBO;
		bool pbr;

		void SetupMesh();
	};
}