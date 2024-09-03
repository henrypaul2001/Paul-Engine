#pragma once
#include "Shader.h"
#include <iostream>
#include <vector>
#include <unordered_map>
#include "ComponentCollisionAABB.h"
#include "ScopeTimer.h"
#include "MeshData.h"
namespace Engine {
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

		~Texture() { glDeleteTextures(1, &id); }
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
		
		glm::vec2 textureScaling = glm::vec2(1.0f);
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

		glm::vec2 textureScaling = glm::vec2(1.0f);
	};

	class Mesh
	{
	public:
		static const std::unordered_map<TextureTypes, std::string> TextureTypeToString;

		Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, PBRMaterial* pbrMaterial);
		Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, Material* material);
		~Mesh();

		void ApplyMaterial(Material* material);
		void ApplyMaterial(PBRMaterial* pbrMaterial);

		void SetDrawPrimitive(GLenum drawPrimitive) { this->drawPrimitive = drawPrimitive; }
		Material* GetMaterial() { return material; }
		PBRMaterial* GetPBRMaterial() { return PBRmaterial; }

		void Draw(Shader& shader, bool pbr, bool ignoreCulling = true, int instanceNum = 0, const unsigned int instanceVAO = 0);
		void DrawWithNoMaterial(int instanceNum = 0, const unsigned int instanceVAO = 0, bool ignoreCulling = true);

		AABBPoints& GetGeometryAABB() { return geometryAABB; }

		const MeshData& GetMeshData() const { return *meshData; }

		void SetIsVisible(const bool isVisible) { this->isVisible = isVisible; }
		const bool IsVisible() const { return isVisible; }
	private:
		GLenum drawPrimitive;
		Material* material;
		PBRMaterial* PBRmaterial;
		AABBPoints geometryAABB;

		MeshData* meshData;

		const std::unordered_map<std::string, unsigned int>* textureSlots;

		bool isVisible;

		void SetupGeometryAABB();
	};
}