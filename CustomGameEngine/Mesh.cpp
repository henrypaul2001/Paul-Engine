#include "Mesh.h"
#include "ResourceManager.h"
namespace Engine {
	const std::unordered_map<TextureTypes, std::string> Mesh::TextureTypeToString {
		{ TEXTURE_NONE, "TEXTURE_NONE" },
		{ TEXTURE_DIFFUSE, "TEXTURE_DIFFUSE" },
		{ TEXTURE_NORMAL, "TEXTURE_NORMAL" },
		{ TEXTURE_METALLIC, "TEXTURE_METALLIC" },
		{ TEXTURE_DISPLACE, "TEXTURE_DISPLACE" },
		{ TEXTURE_AO, "TEXTURE_AO" },
		{ TEXTURE_SPECULAR, "TEXTURE_SPECULAR" },
		{ TEXTURE_HEIGHT, "TEXTURE_HEIGHT" },
		{ TEXTURE_ALBEDO, "TEXTURE_ALBEDO" },
		{ TEXTURE_ROUGHNESS, "TEXTURE_ROUGHNESS" },
		{ TEXTURE_OPACITY, "TEXTURE_OPACITY" }
	};

	Mesh::Mesh(MeshData* meshData)
	{
		isVisible = true;
		drawPrimitive = GL_TRIANGLES;
		this->meshData = meshData;
		SetupGeometryAABB();
	}

	Mesh::Mesh(MeshData* meshData, const std::vector<AbstractMaterial*>& materials)
	{
		this->meshData = meshData;
		this->meshMaterials = materials;

		drawPrimitive = GL_TRIANGLES;
		SetupGeometryAABB();
	}

	Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, const std::vector<AbstractMaterial*>& materials)
	{
		this->meshMaterials = materials;
		isVisible = true;
		drawPrimitive = GL_TRIANGLES;
		meshData = new MeshData(vertices, indices);
		SetupGeometryAABB();
	}

	Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices)
	{
		isVisible = true;
		drawPrimitive = GL_TRIANGLES;
		meshData = new MeshData(vertices, indices);
		SetupGeometryAABB();
	}

	Mesh::~Mesh()
	{
	}

	void Mesh::Draw(Shader& shader, bool pbr, bool ignoreCulling, int instanceNum, const unsigned int instanceVAO)
	{
		SCOPE_TIMER("Mesh::Draw");
		bool visible = ignoreCulling;
		
		if (!ignoreCulling) {
			visible = isVisible;
		}

		if (visible) {
			textureSlots = &ResourceManager::GetInstance()->GetTextureSlotLookupMap();
			AbstractMaterial* firstMaterial = meshMaterials[0];

			if (!firstMaterial) { if (pbr) { firstMaterial = ResourceManager::GetInstance()->DefaultMaterialPBR(); } else { firstMaterial = ResourceManager::GetInstance()->DefaultMaterial(); } }

			std::string number;
			std::string name;
			std::string nameAndNumber;

			// Set base material properties
			// ----------------------------
			shader.setBool("material.useNormalMap", false);
			shader.setBool("material.useHeightMap", false);
			shader.setBool("material.useOpacityMap", false);

			shader.setFloat("material.HEIGHT_SCALE", firstMaterial->height_scale);
			shader.setFloat("material.shadowCastAlphaDiscardThreshold", firstMaterial->shadowCastAlphaDiscardThreshold);
			shader.setVec2("textureScale", firstMaterial->textureScaling);

			unsigned int normalNr = 1;
			unsigned int heightNr = 1;
			unsigned int opacityNr = 1;

			// normal maps
			for (int i = 0; i < firstMaterial->normalMaps.size(); i++) {
				name = TextureTypeToString.at(firstMaterial->normalMaps[i]->type);
				if (name == TextureTypeToString.at(TEXTURE_NORMAL)) {
					nameAndNumber = "material." + name + std::to_string(normalNr);
					glActiveTexture(GL_TEXTURE0 + textureSlots->at(nameAndNumber));
					glBindTexture(GL_TEXTURE_2D, firstMaterial->normalMaps[i]->id);
					normalNr++;
					shader.setBool("material.useNormalMap", true);
				}
			}

			// height maps
			for (int i = 0; i < firstMaterial->heightMaps.size(); i++) {
				name = TextureTypeToString.at(firstMaterial->heightMaps[i]->type);
				if (name == TextureTypeToString.at(TEXTURE_DISPLACE)) {
					nameAndNumber = "material." + name + std::to_string(heightNr);
					glActiveTexture(GL_TEXTURE0 + textureSlots->at(nameAndNumber));
					glBindTexture(GL_TEXTURE_2D, firstMaterial->heightMaps[i]->id);
					heightNr++;
					shader.setBool("material.useHeightMap", true);
				}
			}

			// opacity maps
			if (!firstMaterial->GetUseColourMapAsAlpha()) {
				const std::vector<Texture*>& opacityMaps = firstMaterial->GetOpacityMaps();
				for (int i = 0; i < opacityMaps.size(); i++) {
					name = TextureTypeToString.at(opacityMaps[i]->type);
					if (name == TextureTypeToString.at(TEXTURE_OPACITY) || name == TextureTypeToString.at(TEXTURE_DIFFUSE) || name == TextureTypeToString.at(TEXTURE_ALBEDO)) {
						name = TextureTypeToString.at(TEXTURE_OPACITY);
						nameAndNumber = "material." + name + std::to_string(opacityNr);
						glActiveTexture(GL_TEXTURE0 + textureSlots->at(nameAndNumber));
						glBindTexture(GL_TEXTURE_2D, opacityMaps[i]->id);
						opacityNr++;
						shader.setBool("material.useOpacityMap", true);
					}
				}
			}

			if (!firstMaterial->IsPBR()) {
				Material* material = dynamic_cast<Material*>(firstMaterial);

				shader.setBool("material.useDiffuseMap", false);
				shader.setBool("material.useSpecularMap", false);

				unsigned int diffuseNr = 1;
				unsigned int specularNr = 1;

				shader.setVec3("material.DIFFUSE", material->baseColour);
				shader.setVec3("material.SPECULAR", material->specular);
				shader.setFloat("material.SHININESS", material->shininess);

				// diffuse maps
				for (int i = 0; i < material->baseColourMaps.size(); i++) {
					name = TextureTypeToString.at(material->baseColourMaps[i]->type);
					if (name == TextureTypeToString.at(TEXTURE_DIFFUSE)) {
						nameAndNumber = "material." + name + std::to_string(diffuseNr);
						glActiveTexture(GL_TEXTURE0 + textureSlots->at(nameAndNumber));
						glBindTexture(GL_TEXTURE_2D, material->baseColourMaps[i]->id);
						diffuseNr++;
						shader.setBool("material.useDiffuseMap", true);
					}
				}

				// specular maps
				for (int i = 0; i < material->specularMaps.size(); i++) {
					name = TextureTypeToString.at(material->specularMaps[i]->type);
					if (name == TextureTypeToString.at(TEXTURE_SPECULAR)) {
						nameAndNumber = "material." + name + std::to_string(specularNr);
						glActiveTexture(GL_TEXTURE0 + textureSlots->at(nameAndNumber));
						glBindTexture(GL_TEXTURE_2D, material->specularMaps[i]->id);
						specularNr++;
						shader.setBool("material.useSpecularMap", true);
					}
				}

				// diffuse as opacity
				if (material->GetUseColourMapAsAlpha()) {
					for (int i = 0; i < material->baseColourMaps.size(); i++) {
						name = TextureTypeToString.at(material->baseColourMaps[i]->type);
						if (name == TextureTypeToString.at(TEXTURE_OPACITY) || name == TextureTypeToString.at(TEXTURE_DIFFUSE)) {
							name = TextureTypeToString.at(TEXTURE_OPACITY);
							nameAndNumber = "material." + name + std::to_string(opacityNr);
							glActiveTexture(GL_TEXTURE0 + textureSlots->at(nameAndNumber));
							glBindTexture(GL_TEXTURE_2D, material->baseColourMaps[i]->id);
							opacityNr++;
							shader.setBool("material.useOpacityMap", true);
						}
					}
				}

				glActiveTexture(GL_TEXTURE0);
			}
			else {
				PBRMaterial* PBRmaterial = dynamic_cast<PBRMaterial*>(firstMaterial);

				shader.setBool("material.useAlbedoMap", false);
				shader.setBool("material.useMetallicMap", false);
				shader.setBool("material.useRoughnessMap", false);
				shader.setBool("material.useAoMap", false);

				unsigned int albedoNr = 1;
				unsigned int metallicNr = 1;
				unsigned int roughnessNr = 1;
				unsigned int aoNr = 1;

				shader.setVec3("material.ALBEDO", PBRmaterial->baseColour);
				shader.setFloat("material.METALNESS", PBRmaterial->metallic);
				shader.setFloat("material.ROUGHNESS", PBRmaterial->roughness);
				shader.setFloat("material.AO", PBRmaterial->ao);

				int count = 0;

				// albedo maps
				for (int i = 0; i < PBRmaterial->baseColourMaps.size(); i++) {
					name = TextureTypeToString.at(PBRmaterial->baseColourMaps[i]->type);
					if (name == TextureTypeToString.at(TEXTURE_ALBEDO)) {
						nameAndNumber = "material." + name + std::to_string(albedoNr);
						glActiveTexture(GL_TEXTURE0 + textureSlots->at(nameAndNumber));
						glBindTexture(GL_TEXTURE_2D, PBRmaterial->baseColourMaps[i]->id);
						albedoNr++;
						shader.setBool("material.useAlbedoMap", true);
					}
				}


				// metallic maps
				for (int i = 0; i < PBRmaterial->metallicMaps.size(); i++) {
					name = TextureTypeToString.at(PBRmaterial->metallicMaps[i]->type);
					if (name == TextureTypeToString.at(TEXTURE_METALLIC)) {
						nameAndNumber = "material." + name + std::to_string(metallicNr);
						glActiveTexture(GL_TEXTURE0 + textureSlots->at(nameAndNumber));
						glBindTexture(GL_TEXTURE_2D, PBRmaterial->metallicMaps[i]->id);
						metallicNr++;
						shader.setBool("material.useMetallicMap", true);
					}
				}

				// roughness maps
				for (int i = 0; i < PBRmaterial->roughnessMaps.size(); i++) {
					name = TextureTypeToString.at(PBRmaterial->roughnessMaps[i]->type);
					if (name == TextureTypeToString.at(TEXTURE_ROUGHNESS)) {
						nameAndNumber = "material." + name + std::to_string(roughnessNr);
						glActiveTexture(GL_TEXTURE0 + textureSlots->at(nameAndNumber));
						glBindTexture(GL_TEXTURE_2D, PBRmaterial->roughnessMaps[i]->id);
						roughnessNr++;
						shader.setBool("material.useRoughnessMap", true);
					}
				}

				// ao maps
				for (int i = 0; i < PBRmaterial->aoMaps.size(); i++) {
					name = TextureTypeToString.at(PBRmaterial->aoMaps[i]->type);
					if (name == TextureTypeToString.at(TEXTURE_AO)) {
						nameAndNumber = "material." + name + std::to_string(aoNr);
						glActiveTexture(GL_TEXTURE0 + textureSlots->at(nameAndNumber));
						glBindTexture(GL_TEXTURE_2D, PBRmaterial->aoMaps[i]->id);
						aoNr++;
						shader.setBool("material.useAoMap", true);
					}
				}

				// albedo as opacity
				if (PBRmaterial->GetUseColourMapAsAlpha()) {
					for (int i = 0; i < PBRmaterial->baseColourMaps.size(); i++) {
						name = TextureTypeToString.at(PBRmaterial->baseColourMaps[i]->type);
						if (name == TextureTypeToString.at(TEXTURE_OPACITY) || name == TextureTypeToString.at(TEXTURE_ALBEDO)) {
							name = TextureTypeToString.at(TEXTURE_OPACITY);
							nameAndNumber = "material." + name + std::to_string(opacityNr);
							glActiveTexture(GL_TEXTURE0 + textureSlots->at(nameAndNumber));
							glBindTexture(GL_TEXTURE_2D, PBRmaterial->baseColourMaps[i]->id);
							opacityNr++;
							shader.setBool("material.useOpacityMap", true);
						}
					}
				}

				glActiveTexture(GL_TEXTURE0);
			}

			// draw
			meshData->DrawMeshData(instanceNum, drawPrimitive, instanceVAO);
			glActiveTexture(GL_TEXTURE0);
		}
	}

	void Mesh::DrawWithNoMaterial(int instanceNum, const unsigned int instanceVAO, bool ignoreCulling)
	{
		SCOPE_TIMER("Mesh::DrawWithNoMaterial");
		bool visible = ignoreCulling;

		if (!ignoreCulling) {
			visible = isVisible;
		}

		if (visible) {
			// draw
			meshData->DrawMeshData(instanceNum, drawPrimitive, instanceVAO);
			glActiveTexture(GL_TEXTURE0);
		}
	}

	void Mesh::SetupGeometryAABB()
	{
		const std::vector<Vertex>& vertices = meshData->GetVertices();
		float minX = vertices[0].Position.x, minY = vertices[0].Position.y, minZ = vertices[0].Position.z, maxX = vertices[0].Position.x, maxY = vertices[0].Position.y, maxZ = vertices[0].Position.z;
		for (const Vertex& v : vertices) {
			const float x = v.Position.x, y = v.Position.y, z = v.Position.z;
			if (x < minX) { minX = x; }
			else if (x > maxX) { maxX = x; }

			if (y < minY) { minY = y; }
			else if (y > maxY) { maxY = y; }

			if (z < minZ) { minZ = z; }
			else if (z > maxZ) { maxZ = z; }
		}

		geometryAABB = AABBPoints(minX, minY, minZ, maxX, maxY, maxZ);
	}
}