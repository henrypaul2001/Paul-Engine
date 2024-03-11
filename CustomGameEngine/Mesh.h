#pragma once
#include "Shader.h"
#include <iostream>
#include <vector>
namespace Engine {
	enum TextureTypes {
		TEXTURE_NONE,
		TEXTURE_DIFFUSE,
		TEXTURE_NORMAL,
		TEXTURE_METAL,
		TEXTURE_DISPLACE,
		TEXTURE_AO,
		TEXTURE_SPECULAR,
		TEXTURE_HEIGHT,
		TEXTURE_ALBEDO,
		TEXTURE_ROUGHNESS,
		TEXTURE_OPACITY
	};

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
		case TEXTURE_OPACITY:
			return "TEXTURE_OPACITY";
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

	struct Material {
		std::vector<Texture*> diffuseMaps;
		std::vector<Texture*> specularMaps;
		std::vector<Texture*> normalMaps;
		std::vector<Texture*> heightMaps;
		std::vector<Texture*> opacityMaps;

		float shininess;
		float height_scale;

		glm::vec3 diffuse;
		glm::vec3 specular;

		bool isTransparent;
		// I have no idea how to delete this properly without causing an error. Come back later
	};

	class Mesh
	{
	public:
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		unsigned int VAO;

		Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture*> textures, bool pbr);
		Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, Material* material, bool pbr);
		~Mesh();

		void ApplyMaterial(Material* material);
		void SetDrawPrimitive(GLenum drawPrimitive) { this->drawPrimitive = drawPrimitive; }
		Material* GetMaterial() { return material; }

		bool PBR() { return pbr; }

		void Draw(Shader& shader);
	private:
		GLenum drawPrimitive;
		unsigned int VBO, EBO;
		Material* material;
		bool pbr;

		void SetupMesh();
	};
}