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

	Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, PBRMaterial* pbrMaterial)
	{
		this->vertices = vertices;
		this->indices = indices;
		this->PBRmaterial = pbrMaterial;
		SetupMesh();
	}

	Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, Material* material)
	{
		this->vertices = vertices;
		this->indices = indices;
		this->material = material;
		SetupMesh();
	}

	Mesh::~Mesh()
	{
	}

	void Mesh::ApplyMaterial(Material* material)
	{
		this->material = material;
	}

	void Mesh::ApplyMaterial(PBRMaterial* pbrMaterial)
	{
		this->PBRmaterial = pbrMaterial;
	}

	void Mesh::Draw(Shader& shader, bool pbr, int instanceNum)
	{
		textureSlots = &ResourceManager::GetInstance()->GetTextureSlotLookupMap();
		int offset = 10;
		if (!pbr) {
			shader.setBool("material.useDiffuseMap", false);
			shader.setBool("material.useSpecularMap", false);
			shader.setBool("material.useNormalMap", false);
			shader.setBool("material.useHeightMap", false);
			shader.setBool("material.useOpacityMap", false);

			unsigned int diffuseNr = 1;
			unsigned int specularNr = 1;
			unsigned int normalNr = 1;
			unsigned int heightNr = 1;
			unsigned int opacityNr = 1;

			std::string number;
			std::string name;
			std::string nameAndNumber;

			int count = 0;

			shader.setVec3("material.DIFFUSE", material->diffuse);
			shader.setVec3("material.SPECULAR", material->specular);
			shader.setFloat("material.SHININESS", material->shininess);
			shader.setFloat("material.HEIGHT_SCALE", material->height_scale);
			shader.setFloat("material.shadowCastAlphaDiscardThreshold", material->shadowCastAlphaDiscardThreshold);

			// diffuse maps
			for (int i = 0; i < material->diffuseMaps.size(); i++) {
				name = TextureTypeToString.at(material->diffuseMaps[i]->type);
				if (name == TextureTypeToString.at(TEXTURE_DIFFUSE)) {
					nameAndNumber = "material." + name + std::to_string(diffuseNr);
					glActiveTexture(GL_TEXTURE0 + textureSlots->at(nameAndNumber));
					glBindTexture(GL_TEXTURE_2D, material->diffuseMaps[i]->id);
					diffuseNr++;
					shader.setBool("material.useDiffuseMap", true);
				}
				count++;
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
				count++;
			}

			// normal maps
			for (int i = 0; i < material->normalMaps.size(); i++) {
				name = TextureTypeToString.at(material->normalMaps[i]->type);
				if (name == TextureTypeToString.at(TEXTURE_NORMAL)) {
					nameAndNumber = "material." + name + std::to_string(normalNr);
					glActiveTexture(GL_TEXTURE0 + textureSlots->at(nameAndNumber));
					glBindTexture(GL_TEXTURE_2D, material->normalMaps[i]->id);
					normalNr++;
					shader.setBool("material.useNormalMap", true);
				}
				count++;
			}

			// height maps
			for (int i = 0; i < material->heightMaps.size(); i++) {
				name = TextureTypeToString.at(material->heightMaps[i]->type);
				if (name == TextureTypeToString.at(TEXTURE_DISPLACE)) {
					nameAndNumber = "material." + name + std::to_string(heightNr);
					glActiveTexture(GL_TEXTURE0 + textureSlots->at(nameAndNumber));
					glBindTexture(GL_TEXTURE_2D, material->heightMaps[i]->id);
					heightNr++;
					shader.setBool("material.useHeightMap", true);
				}
				count++;
			}

			// opacity maps
			if (!material->useDiffuseAlphaAsOpacity) {
				for (int i = 0; i < material->opacityMaps.size(); i++) {
					name = TextureTypeToString.at(material->opacityMaps[i]->type);
					if (name == TextureTypeToString.at(TEXTURE_OPACITY) || name == TextureTypeToString.at(TEXTURE_DIFFUSE)) {
						name = TextureTypeToString.at(TEXTURE_OPACITY);
						nameAndNumber = "material." + name + std::to_string(opacityNr);
						glActiveTexture(GL_TEXTURE0 + textureSlots->at(nameAndNumber));
						glBindTexture(GL_TEXTURE_2D, material->opacityMaps[i]->id);
						opacityNr++;
						shader.setBool("material.useOpacityMap", true);
					}
					count++;
				}
			}
			else {
				for (int i = 0; i < material->diffuseMaps.size(); i++) {
					name = TextureTypeToString.at(material->diffuseMaps[i]->type);
					if (name == TextureTypeToString.at(TEXTURE_OPACITY) || name == TextureTypeToString.at(TEXTURE_DIFFUSE)) {
						name = TextureTypeToString.at(TEXTURE_OPACITY);
						nameAndNumber = "material." + name + std::to_string(opacityNr);
						glActiveTexture(GL_TEXTURE0 + textureSlots->at(nameAndNumber));
						glBindTexture(GL_TEXTURE_2D, material->diffuseMaps[i]->id);
						opacityNr++;
						shader.setBool("material.useOpacityMap", true);
					}
					count++;
				}
			}

			glActiveTexture(GL_TEXTURE0);
		}
		else {
			shader.setBool("material.useAlbedoMap", false);
			shader.setBool("material.useNormalMap", false);
			shader.setBool("material.useMetallicMap", false);
			shader.setBool("material.useRoughnessMap", false);
			shader.setBool("material.useAoMap", false);
			shader.setBool("material.useHeightMap", false);
			shader.setBool("material.useOpacityMap", false);

			unsigned int albedoNr = 1;
			unsigned int normalNr = 1;
			unsigned int metallicNr = 1;
			unsigned int roughnessNr = 1;
			unsigned int aoNr = 1;
			unsigned int heightNr = 1;
			unsigned int opacityNr = 1;

			std::string number;
			std::string name;
			std::string nameAndNumber;
			
			shader.setVec3("material.ALBEDO", PBRmaterial->albedo);
			shader.setFloat("material.METALNESS", PBRmaterial->metallic);
			shader.setFloat("material.ROUGHNESS", PBRmaterial->roughness);
			shader.setFloat("material.AO", PBRmaterial->ao);
			shader.setFloat("material.HEIGHT_SCALE", PBRmaterial->height_scale);
			shader.setFloat("material.shadowCastAlphaDiscardThreshold", PBRmaterial->shadowCastAlphaDiscardThreshold);

			int count = 0;

			// albedo maps
			for (int i = 0; i < PBRmaterial->albedoMaps.size(); i++) {
				name = TextureTypeToString.at(PBRmaterial->albedoMaps[i]->type);
				if (name == TextureTypeToString.at(TEXTURE_ALBEDO)) {
					nameAndNumber = "material." + name + std::to_string(albedoNr);
					glActiveTexture(GL_TEXTURE0 + textureSlots->at(nameAndNumber));
					glBindTexture(GL_TEXTURE_2D, PBRmaterial->albedoMaps[i]->id);
					albedoNr++;
					shader.setBool("material.useAlbedoMap", true);
				}
				count++;
			}

			// normal maps
			for (int i = 0; i < PBRmaterial->normalMaps.size(); i++) {
				name = TextureTypeToString.at(PBRmaterial->normalMaps[i]->type);
				if (name == TextureTypeToString.at(TEXTURE_NORMAL)) {
					nameAndNumber = "material." + name + std::to_string(normalNr);
					glActiveTexture(GL_TEXTURE0 + textureSlots->at(nameAndNumber));
					glBindTexture(GL_TEXTURE_2D, PBRmaterial->normalMaps[i]->id);
					normalNr++;
					shader.setBool("material.useNormalMap", true);
				}
				count++;
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
				count++;
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
				count++;
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
				count++;
			}

			// height maps
			for (int i = 0; i < PBRmaterial->heightMaps.size(); i++) {
				glActiveTexture(GL_TEXTURE0 + count + offset);
				name = TextureTypeToString.at(PBRmaterial->heightMaps[i]->type);
				if (name == TextureTypeToString.at(TEXTURE_DISPLACE)) {
					nameAndNumber = "material." + name + std::to_string(heightNr);
					glActiveTexture(GL_TEXTURE0 + textureSlots->at(nameAndNumber));
					glBindTexture(GL_TEXTURE_2D, PBRmaterial->heightMaps[i]->id);
					heightNr++;
					shader.setBool("material.useHeightMap", true);
				}
				count++;
			}

			// opacity maps
			if (!PBRmaterial->useDiffuseAlphaAsOpacity) {
				for (int i = 0; i < PBRmaterial->opacityMaps.size(); i++) {
					name = TextureTypeToString.at(PBRmaterial->opacityMaps[i]->type);
					if (name == TextureTypeToString.at(TEXTURE_OPACITY) || name == TextureTypeToString.at(TEXTURE_ALBEDO)) {
						name = TextureTypeToString.at(TEXTURE_OPACITY);
						nameAndNumber = "material." + name + std::to_string(opacityNr);
						glActiveTexture(GL_TEXTURE0 + textureSlots->at(nameAndNumber));
						glBindTexture(GL_TEXTURE_2D, PBRmaterial->opacityMaps[i]->id);
						opacityNr++;
						shader.setBool("material.useOpacityMap", true);
					}
					count++;
				}
			}
			else {
				for (int i = 0; i < PBRmaterial->albedoMaps.size(); i++) {
					name = TextureTypeToString.at(PBRmaterial->albedoMaps[i]->type);
					if (name == TextureTypeToString.at(TEXTURE_OPACITY) || name == TextureTypeToString.at(TEXTURE_ALBEDO)) {
						name = TextureTypeToString.at(TEXTURE_OPACITY);
						nameAndNumber = "material." + name + std::to_string(opacityNr);
						glActiveTexture(GL_TEXTURE0 + textureSlots->at(nameAndNumber));
						glBindTexture(GL_TEXTURE_2D, PBRmaterial->albedoMaps[i]->id);
						opacityNr++;
						shader.setBool("material.useOpacityMap", true);
					}
					count++;
				}
			}

			glActiveTexture(GL_TEXTURE0);
		}

		// draw
		glBindVertexArray(VAO);
		if (instanceNum == 0) {
			glDrawElements(drawPrimitive, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
		}
		else if (instanceNum > 0) {
			glDrawElementsInstanced(drawPrimitive, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0, instanceNum);
		}

		glBindVertexArray(0);
		glActiveTexture(GL_TEXTURE0);
	}

	void Mesh::DrawWithNoMaterial(int instanceNum)
	{
		// draw
		glBindVertexArray(VAO);
		if (instanceNum == 0) {
			glDrawElements(drawPrimitive, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
		}
		else if (instanceNum > 0) {
			glDrawElementsInstanced(drawPrimitive, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0, instanceNum);
		}

		glBindVertexArray(0);
		glActiveTexture(GL_TEXTURE0);
	}

	void Mesh::SetupMesh()
	{
		drawPrimitive = GL_TRIANGLES;
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

		// vertex positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

		// normals
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

		// texture coords
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

		// vertex tangent
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));

		// vertex bitangent
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

		// ids
		glEnableVertexAttribArray(5);
		glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, BoneIDs));

		// weights
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, BoneWeights));
		glBindVertexArray(0);

		// 7, 8, 9, 10 reserved for instancing
	}
}