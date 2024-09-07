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
			SetUseColourMapAsAlpha(false);
		}
		AbstractMaterial(const std::vector<Texture*>& baseColourMaps, const bool useDiffuseAlpha = false) : baseColour(glm::vec3(1.0f)), shadowCastAlphaDiscardThreshold(0.0f), baseColourMaps(baseColourMaps) {
			SetAlpha(1.0f);
			SetUseColourMapAsAlpha(useDiffuseAlpha);
		}
		AbstractMaterial(const std::vector<Texture*>& baseColourMaps, const std::vector<Texture*>& normalMaps) : baseColour(glm::vec3(1.0f)), shadowCastAlphaDiscardThreshold(0.0f), baseColourMaps(baseColourMaps), normalMaps(normalMaps) {
			SetAlpha(1.0f);
			SetUseColourMapAsAlpha(false);
		}
		AbstractMaterial(const std::vector<Texture*>& baseColourMaps, const std::vector<Texture*>& normalMaps, const std::vector<Texture*>& heightMaps) : baseColour(glm::vec3(1.0f)), shadowCastAlphaDiscardThreshold(0.0f), baseColourMaps(baseColourMaps), normalMaps(normalMaps), heightMaps(heightMaps) {
			SetAlpha(1.0f);
			SetUseColourMapAsAlpha(false);
		}
		AbstractMaterial(const std::vector<Texture*>& baseColourMaps, const std::vector<Texture*>& normalMaps, const std::vector<Texture*>& heightMaps, const std::vector<Texture*>& opacityMaps) : baseColour(glm::vec3(1.0f)), shadowCastAlphaDiscardThreshold(0.0f), baseColourMaps(baseColourMaps), normalMaps(normalMaps), heightMaps(heightMaps), opacityMaps(opacityMaps) {
			SetAlpha(1.0f);
			SetUseColourMapAsAlpha(false);
			if (opacityMaps.size() > 0) { isTransparent = true; }
		}
		virtual ~AbstractMaterial() {}

		const float GetAlpha() const { return alpha; }
		const bool GetIsTransparent() const { return isTransparent; }
		const bool GetUseColourMapAsAlpha() const { return useColourMapAlphaAsOpacity; }
		void SetAlpha(const float alpha) {
			this->alpha = alpha;
			this->isTransparent = (alpha < 1.0f);
			if (opacityMaps.size() > 0) { isTransparent = true; }
		}
		void SetUseColourMapAsAlpha(const bool newValue) {
			this->useColourMapAlphaAsOpacity = newValue;
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
			SetUseColourMapAsAlpha(useColourMapAlphaAsOpacity);
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
		bool useColourMapAlphaAsOpacity;
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

	class Model;
	class Mesh
	{
	public:
		static const std::unordered_map<TextureTypes, std::string> TextureTypeToString;

		Mesh(MeshData* meshData);
		Mesh(MeshData* meshData, const std::vector<AbstractMaterial*>& materials);
		Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, const std::vector<AbstractMaterial*>& materials);
		Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices);
		~Mesh();

		AbstractMaterial* GetMaterial(const unsigned int materialIndex = 0);
		void SetMaterials(const std::vector<AbstractMaterial*>& materials) { this->meshMaterials = materials; }

		void SetDrawPrimitive(GLenum drawPrimitive) { this->drawPrimitive = drawPrimitive; }

		void Draw(Shader& shader, bool pbr, int instanceNum = 0, const unsigned int instanceVAO = 0);
		void DrawWithNoMaterial(int instanceNum = 0, const unsigned int instanceVAO = 0);

		AABBPoints& GetGeometryAABB() { return geometryAABB; }

		const MeshData& GetMeshData() const { return *meshData; }

		Model* GetOwner() { return owner; }
		const Model* GetOwnerConst() const { return owner; }
		void SetOwner(Model* newOwner) { this->owner = newOwner; }

		const unsigned int GetLocalMeshID() const { return localMeshID; }
		void SetLocalMeshID(const unsigned int newID) { this->localMeshID = newID; }
	private:
		GLenum drawPrimitive;
		std::vector<AbstractMaterial*> meshMaterials;
		AABBPoints geometryAABB;

		MeshData* meshData;
		Model* owner;

		unsigned int localMeshID;

		const std::unordered_map<std::string, unsigned int>* textureSlots;

		void SetupGeometryAABB();
	};
}