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

	struct AbstractMaterial {
	public:
		AbstractMaterial(const glm::vec3& baseColour, const float alpha = 1.0f, const float shadowCastAlphaDiscardThreshold = 0.0f) : baseColour(baseColour), shadowCastAlphaDiscardThreshold(shadowCastAlphaDiscardThreshold) {
			SetAlpha(alpha);
			SetUseDiffuseAsAlpha(false);
		}
		AbstractMaterial(const std::vector<Texture*>& baseColourMaps, const bool useDiffuseAlpha = false) : baseColour(glm::vec3(1.0f)), shadowCastAlphaDiscardThreshold(0.0f), baseColourMaps(baseColourMaps) {
			SetAlpha(1.0f);
			SetUseDiffuseAsAlpha(useDiffuseAlpha);
		}
		AbstractMaterial(const std::vector<Texture*>& baseColourMaps, const std::vector<Texture*>& normalMaps) : baseColour(glm::vec3(1.0f)), shadowCastAlphaDiscardThreshold(0.0f), baseColourMaps(baseColourMaps), normalMaps(normalMaps) {
			SetAlpha(1.0f);
			SetUseDiffuseAsAlpha(false);
		}
		AbstractMaterial(const std::vector<Texture*>& baseColourMaps, const std::vector<Texture*>& normalMaps, const std::vector<Texture*>& heightMaps) : baseColour(glm::vec3(1.0f)), shadowCastAlphaDiscardThreshold(0.0f), baseColourMaps(baseColourMaps), normalMaps(normalMaps), heightMaps(heightMaps) {
			SetAlpha(1.0f);
			SetUseDiffuseAsAlpha(false);
		}
		AbstractMaterial(const std::vector<Texture*>& baseColourMaps, const std::vector<Texture*>& normalMaps, const std::vector<Texture*>& heightMaps, const std::vector<Texture*>& opacityMaps) : baseColour(glm::vec3(1.0f)), shadowCastAlphaDiscardThreshold(0.0f), baseColourMaps(baseColourMaps), normalMaps(normalMaps), heightMaps(heightMaps), opacityMaps(opacityMaps) {
			SetAlpha(1.0f);
			SetUseDiffuseAsAlpha(false);
			if (opacityMaps.size() > 0) { isTransparent = true; }
		}
		
		const float GetAlpha() const { return alpha; }
		const bool GetIsTransparent() const { return isTransparent; }
		const bool GetUseDiffuseAsAlpha() const { return useDiffuseAlphaAsOpacity; }
		void SetAlpha(const float alpha) {
			this->alpha = alpha;
			this->isTransparent = (alpha < 1.0f);
			if (opacityMaps.size() > 0) { isTransparent = true; }
		}
		void SetUseDiffuseAsAlpha(const bool newValue) { 
			this->useDiffuseAlphaAsOpacity = newValue;
			isTransparent = newValue;
			SetAlpha(alpha);
			if (opacityMaps.size() > 0) { isTransparent = true; }
		}
		const bool IsPBR() const { return isPBR; }

		void PushOpacityMap(Texture* opacityMap) {
			opacityMaps.push_back(opacityMap);
			isTransparent = true;
		}
		void RemoveOpacityMapAtIndex(const unsigned int index) {
			opacityMaps.erase(opacityMaps.begin() + index);
			SetAlpha(alpha);
			SetUseDiffuseAsAlpha(useDiffuseAlphaAsOpacity);
		}

		const std::vector<Texture*>& GetOpacityMaps() const { return opacityMaps; }

		float height_scale = 0.5f;
		float shadowCastAlphaDiscardThreshold = 0.5f;

		glm::vec2 textureScaling = glm::vec2(1.0f);

		glm::vec3 baseColour;

		std::vector<Texture*> baseColourMaps;
		std::vector<Texture*> normalMaps;
		std::vector<Texture*> heightMaps;

	protected:
		float alpha;
		bool isTransparent;
		bool useDiffuseAlphaAsOpacity;
		bool isPBR;

		std::vector<Texture*> opacityMaps;
	};

	struct Material : AbstractMaterial {
		Material(const glm::vec3& diffuse = glm::vec3(1.0f), const glm::vec3& specular = glm::vec3(1.0f), const float shininess = 1.0f, const float alpha = 1.0f, const float shadowCastAlphaDiscardThreshold = 0.0f) : AbstractMaterial(diffuse, alpha, shadowCastAlphaDiscardThreshold), specular(specular), shininess(shininess) {
			isPBR = false;
		}
		Material(const std::vector<Texture*>& diffuseMaps) : AbstractMaterial(diffuseMaps), shininess(1.0f), specular(baseColour) {
			isPBR = false;
		}
		Material(const std::vector<Texture*>& diffuseMaps, const std::vector<Texture*>& specularMaps, const float shininess = 1.0f) : AbstractMaterial(diffuseMaps), specularMaps(specularMaps), shininess(shininess), specular(baseColour) {
			isPBR = false;
		}
		Material(const std::vector<Texture*>& diffuseMaps, const std::vector<Texture*>& specularMaps, const std::vector<Texture*>& normalMaps, const std::vector<Texture*>& heightMaps, const std::vector<Texture*>& opacityMaps, const float shininess = 1.0f) : AbstractMaterial(diffuseMaps, normalMaps, heightMaps, opacityMaps), specularMaps(specularMaps), shininess(shininess), specular(baseColour) {
			isPBR = false;
		}

		std::vector<Texture*> specularMaps;
		float shininess;
		glm::vec3 specular;
	};

	struct PBRMaterial : AbstractMaterial {
		PBRMaterial(const glm::vec3& albedo = glm::vec3(10.0f), const float metallic = 0.5f, const float roughness = 0.5f, const float ao = 1.0f, const float alpha = 1.0f, const float shadowCastAlphaDiscardThreshold = 0.0f) : AbstractMaterial(albedo, alpha, shadowCastAlphaDiscardThreshold),
			metallic(metallic), roughness(roughness), ao(ao) {
			isPBR = true;
		}
		PBRMaterial(const std::vector<Texture*>& albedoMaps, const std::vector<Texture*>& normalMaps, const std::vector<Texture*>& metallicMaps, const std::vector<Texture*>& roughnessMaps, const std::vector<Texture*>& aoMaps, 
			const std::vector<Texture*>& heightMaps, const std::vector<Texture*>& opacityMaps) :
			AbstractMaterial(albedoMaps, normalMaps, heightMaps, opacityMaps), metallicMaps(metallicMaps), roughnessMaps(roughnessMaps), aoMaps(aoMaps), metallic(0.5f), roughness(0.5f), ao(1.0f) {
			isPBR = true;
		}

		std::vector<Texture*> metallicMaps;
		std::vector<Texture*> roughnessMaps;
		std::vector<Texture*> aoMaps;

		float metallic;
		float roughness;
		float ao;
	};

	class Mesh
	{
	public:
		static const std::unordered_map<TextureTypes, std::string> TextureTypeToString;

		Mesh(MeshData* meshData);
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
		AbstractMaterial* meshMaterial;
		Material* material;
		PBRMaterial* PBRmaterial;
		AABBPoints geometryAABB;

		MeshData* meshData;

		const std::unordered_map<std::string, unsigned int>* textureSlots;

		bool isVisible;

		void SetupGeometryAABB();
	};
}