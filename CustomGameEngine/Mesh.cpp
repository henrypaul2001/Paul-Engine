#include "Mesh.h"

namespace Engine {
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
		if (material != nullptr) {
			// TODO: COME BACK TO THIS LATER DONT FORGET
			//delete material;
		}
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
		int offset = 18;
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

			int count = 0;

			shader.setVec3("material.DIFFUSE", material->diffuse);
			shader.setVec3("material.SPECULAR", material->specular);
			shader.setFloat("material.SHININESS", material->shininess);
			shader.setFloat("material.HEIGHT_SCALE", material->height_scale);

			// diffuse maps
			for (int i = 0; i < material->diffuseMaps.size(); i++) {
				glActiveTexture(GL_TEXTURE0 + count + offset);
				name = ConvertTextureTypeToString(material->diffuseMaps[i]->type);
				if (name == ConvertTextureTypeToString(TEXTURE_DIFFUSE)) {
					shader.setInt(("material." + name + std::to_string(diffuseNr)).c_str(), count + offset);
					glBindTexture(GL_TEXTURE_2D, material->diffuseMaps[i]->id);
					diffuseNr++;
					shader.setBool("material.useDiffuseMap", true);
				}
				count++;
			}

			// specular maps
			for (int i = 0; i < material->specularMaps.size(); i++) {
				glActiveTexture(GL_TEXTURE0 + count + offset);
				name = ConvertTextureTypeToString(material->specularMaps[i]->type);
				if (name == ConvertTextureTypeToString(TEXTURE_SPECULAR)) {
					shader.setInt(("material." + name + std::to_string(specularNr)).c_str(), count + offset);
					glBindTexture(GL_TEXTURE_2D, material->specularMaps[i]->id);
					specularNr++;
					shader.setBool("material.useSpecularMap", true);
				}
				count++;
			}

			// normal maps
			for (int i = 0; i < material->normalMaps.size(); i++) {
				glActiveTexture(GL_TEXTURE0 + count + offset);
				name = ConvertTextureTypeToString(material->normalMaps[i]->type);
				if (name == ConvertTextureTypeToString(TEXTURE_NORMAL)) {
					shader.setInt(("material." + name + std::to_string(normalNr)).c_str(), count + offset);
					glBindTexture(GL_TEXTURE_2D, material->normalMaps[i]->id);
					normalNr++;
					shader.setBool("material.useNormalMap", true);
				}
				count++;
			}

			// height maps
			for (int i = 0; i < material->heightMaps.size(); i++) {
				glActiveTexture(GL_TEXTURE0 + count + offset);
				name = ConvertTextureTypeToString(material->heightMaps[i]->type);
				if (name == ConvertTextureTypeToString(TEXTURE_DISPLACE)) {
					shader.setInt(("material." + name + std::to_string(heightNr)).c_str(), count + offset);
					glBindTexture(GL_TEXTURE_2D, material->heightMaps[i]->id);
					heightNr++;
					shader.setBool("material.useHeightMap", true);
				}
				count++;
			}

			// opacity maps
			for (int i = 0; i < material->opacityMaps.size(); i++) {
				glActiveTexture(GL_TEXTURE0 + count + offset);
				name = ConvertTextureTypeToString(material->opacityMaps[i]->type);
				if (name == ConvertTextureTypeToString(TEXTURE_OPACITY)) {
					shader.setInt(("material." + name + std::to_string(opacityNr)).c_str(), count + offset);
					glBindTexture(GL_TEXTURE_2D, material->opacityMaps[i]->id);
					opacityNr++;
					shader.setBool("material.useOpacityMap", true);
				}
				count++;
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
			
			shader.setVec3("material.ALBEDO", PBRmaterial->albedo);
			shader.setFloat("material.METALNESS", PBRmaterial->metallic);
			shader.setFloat("material.ROUGHNESS", PBRmaterial->roughness);
			shader.setFloat("material.AO", PBRmaterial->ao);
			shader.setFloat("material.HEIGHT_SCALE", PBRmaterial->height_scale);

			int count = 0;

			// albedo maps
			for (int i = 0; i < PBRmaterial->albedoMaps.size(); i++) {
				glActiveTexture(GL_TEXTURE0 + count + offset);
				name = ConvertTextureTypeToString(PBRmaterial->albedoMaps[i]->type);
				if (name == ConvertTextureTypeToString(TEXTURE_ALBEDO)) {
					shader.setInt(("material." + name + std::to_string(albedoNr)).c_str(), count + offset);
					glBindTexture(GL_TEXTURE_2D, PBRmaterial->albedoMaps[i]->id);
					albedoNr++;
					shader.setBool("material.useAlbedoMap", true);
				}
				count++;
			}

			// normal maps
			for (int i = 0; i < PBRmaterial->normalMaps.size(); i++) {
				glActiveTexture(GL_TEXTURE0 + count + offset);
				name = ConvertTextureTypeToString(PBRmaterial->normalMaps[i]->type);
				if (name == ConvertTextureTypeToString(TEXTURE_NORMAL)) {
					shader.setInt(("material." + name + std::to_string(normalNr)).c_str(), count + offset);
					glBindTexture(GL_TEXTURE_2D, PBRmaterial->normalMaps[i]->id);
					normalNr++;
					shader.setBool("material.useNormalMap", true);
				}
				count++;
			}

			// metallic maps
			for (int i = 0; i < PBRmaterial->metallicMaps.size(); i++) {
				glActiveTexture(GL_TEXTURE0 + count + offset);
				name = ConvertTextureTypeToString(PBRmaterial->metallicMaps[i]->type);
				if (name == ConvertTextureTypeToString(TEXTURE_METALLIC)) {
					shader.setInt(("material." + name + std::to_string(metallicNr)).c_str(), count + offset);
					glBindTexture(GL_TEXTURE_2D, PBRmaterial->metallicMaps[i]->id);
					metallicNr++;
					shader.setBool("material.useMetallicMap", true);
				}
				count++;
			}

			// roughness maps
			for (int i = 0; i < PBRmaterial->roughnessMaps.size(); i++) {
				glActiveTexture(GL_TEXTURE0 + count + offset);
				name = ConvertTextureTypeToString(PBRmaterial->roughnessMaps[i]->type);
				if (name == ConvertTextureTypeToString(TEXTURE_ROUGHNESS)) {
					shader.setInt(("material." + name + std::to_string(roughnessNr)).c_str(), count + offset);
					glBindTexture(GL_TEXTURE_2D, PBRmaterial->roughnessMaps[i]->id);
					roughnessNr++;
					shader.setBool("material.useRoughnessMap", true);
				}
				count++;
			}
			
			// ao maps
			for (int i = 0; i < PBRmaterial->aoMaps.size(); i++) {
				glActiveTexture(GL_TEXTURE0 + count + offset);
				name = ConvertTextureTypeToString(PBRmaterial->aoMaps[i]->type);
				if (name == ConvertTextureTypeToString(TEXTURE_AO)) {
					shader.setInt(("material." + name + std::to_string(aoNr)).c_str(), count + offset);
					glBindTexture(GL_TEXTURE_2D, PBRmaterial->aoMaps[i]->id);
					aoNr++;
					shader.setBool("material.useAoMap", true);
				}
				count++;
			}

			// height maps
			for (int i = 0; i < PBRmaterial->heightMaps.size(); i++) {
				glActiveTexture(GL_TEXTURE0 + count + offset);
				name = ConvertTextureTypeToString(PBRmaterial->heightMaps[i]->type);
				if (name == ConvertTextureTypeToString(TEXTURE_DISPLACE)) {
					shader.setInt(("material." + name + std::to_string(heightNr)).c_str(), count + offset);
					glBindTexture(GL_TEXTURE_2D, PBRmaterial->heightMaps[i]->id);
					heightNr++;
					shader.setBool("material.useHeightMap", true);
				}
				count++;
			}

			// opacity maps
			for (int i = 0; i < PBRmaterial->opacityMaps.size(); i++) {
				glActiveTexture(GL_TEXTURE0 + count + offset);
				name = ConvertTextureTypeToString(PBRmaterial->opacityMaps[i]->type);
				if (name == ConvertTextureTypeToString(TEXTURE_OPACITY)) {
					shader.setInt(("material." + name + std::to_string(opacityNr)).c_str(), count + offset);
					glBindTexture(GL_TEXTURE_2D, PBRmaterial->opacityMaps[i]->id);
					opacityNr++;
					shader.setBool("material.useOpacityMap", true);
				}
				count++;
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

		// 5 reserved for instancing

		// ids
		glEnableVertexAttribArray(6);
		glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, BoneIDs));

		// weights
		glEnableVertexAttribArray(7);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, BoneWeights));
		glBindVertexArray(0);
	}
}