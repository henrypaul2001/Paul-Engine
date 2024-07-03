#pragma once
#include "Shader.h"
#include <iostream>
#include <vector>
#include <unordered_map>
namespace Engine {

	static constexpr int MaxBoneInfluence() { return 8; }
	static constexpr int MaxBoneCount() { return 200; }

	enum TextureTypes {
		TEXTURE_NONE,
		TEXTURE_DIFFUSE,
		TEXTURE_NORMAL,
		TEXTURE_METALLIC,
		TEXTURE_DISPLACE,
		TEXTURE_AO,
		TEXTURE_SPECULAR,
		TEXTURE_HEIGHT,
		TEXTURE_ALBEDO,
		TEXTURE_ROUGHNESS,
		TEXTURE_OPACITY
	};

	struct Texture {
		unsigned int id;
		TextureTypes type;
		std::string filepath;
	};

	struct Vertex {
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoords;
		glm::vec3 Tangent;
		glm::vec3 Bitangent;

		int BoneIDs[MaxBoneInfluence()] = { -1, -1, -1, -1, -1, -1, -1, -1 };
		float BoneWeights[MaxBoneInfluence()] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

		void AddBoneData(int boneID, float boneWeight) {
			if (boneWeight == 0.0f) {
				// skip bone
				return;
			}

			for (int i = 0; i < MaxBoneInfluence(); i++) {
				if (BoneIDs[i] == -1 && BoneIDs[i] != boneID) {
					// empty bone slot found
					BoneIDs[i] = boneID;
					BoneWeights[i] = boneWeight;
					return;
				}
			}
		}
	};

	struct Material {
		std::vector<Texture*> diffuseMaps;
		std::vector<Texture*> specularMaps;
		std::vector<Texture*> normalMaps;
		std::vector<Texture*> heightMaps;
		std::vector<Texture*> opacityMaps;

		float shininess;
		float height_scale;
		float shadowCastAlphaDiscardThreshold = 0.5f;

		glm::vec3 diffuse;
		glm::vec3 specular;

		bool isTransparent;
		bool useDiffuseAlphaAsOpacity;
		// I have no idea how to delete this properly without causing an error. Come back later
	};

	struct PBRMaterial {
		std::vector<Texture*> albedoMaps;
		std::vector<Texture*> normalMaps;
		std::vector<Texture*> metallicMaps;
		std::vector<Texture*> roughnessMaps;
		std::vector<Texture*> aoMaps;
		std::vector<Texture*> heightMaps;
		std::vector<Texture*> opacityMaps;

		float height_scale;
		float shadowCastAlphaDiscardThreshold = 0.5f;

		glm::vec3 albedo;
		float metallic;
		float roughness;
		float ao;

		bool isTransparent;
		bool useDiffuseAlphaAsOpacity;
	};

	class Mesh
	{
	public:
		static const std::unordered_map<TextureTypes, std::string> TextureTypeToString;

		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		unsigned int VAO;

		Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, PBRMaterial* pbrMaterial);
		Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, Material* material);
		~Mesh();

		void ApplyMaterial(Material* material);
		void ApplyMaterial(PBRMaterial* pbrMaterial);

		void SetDrawPrimitive(GLenum drawPrimitive) { this->drawPrimitive = drawPrimitive; }
		Material* GetMaterial() { return material; }
		PBRMaterial* GetPBRMaterial() { return PBRmaterial; }

		void Draw(Shader& shader, bool pbr, int instanceNum = 0);
		void DrawWithNoMaterial(int instanceNum = 0);
	private:
		GLenum drawPrimitive;
		unsigned int VBO, EBO;
		Material* material;
		PBRMaterial* PBRmaterial;

		void SetupMesh();
	};
}