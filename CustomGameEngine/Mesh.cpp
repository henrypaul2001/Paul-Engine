#include "Mesh.h"

namespace Engine {
	Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture*> textures, bool pbr)
	{
		this->vertices = vertices;
		this->indices = indices;
		this->textures = textures;
		this->pbr = pbr;
		this->material = nullptr;
		SetupMesh();
	}

	Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, Material* material, bool pbr)
	{
		this->vertices = vertices;
		this->indices = indices;
		this->textures = textures;
		this->pbr = pbr;
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

	void Mesh::Draw(Shader& shader)
	{
		int offset = 17;
		if (!pbr) {
			shader.setBool("material.useDiffuseMap", false);
			shader.setBool("material.useSpecularMap", false);
			shader.setBool("material.useNormalMap", false);
			shader.setBool("material.useHeightMap", false);

			unsigned int diffuseNr = 1;
			unsigned int specularNr = 1;
			unsigned int normalNr = 1;
			unsigned int heightNr = 1;

			std::string number;
			std::string name;

			if (material == nullptr) {
				for (unsigned int i = 0; i < textures.size(); i++) {
					glActiveTexture(GL_TEXTURE0 + i + offset);
					name = ConvertTextureTypeToString(textures[i]->type);
					if (name == ConvertTextureTypeToString(TEXTURE_DIFFUSE)) {
						number = std::to_string(diffuseNr++);
						shader.setBool("material.useDiffuseMap", true);
					}
					else if (name == ConvertTextureTypeToString(TEXTURE_SPECULAR)) {
						number = std::to_string(specularNr++);
						shader.setBool("material.useSpecularMap", true);
					}
					else if (name == ConvertTextureTypeToString(TEXTURE_NORMAL)) {
						number = std::to_string(normalNr++);
						shader.setBool("material.useNormalMap", true);
					}
					else if (name == ConvertTextureTypeToString(TEXTURE_HEIGHT)) {
						number = std::to_string(heightNr++);
						shader.setBool("material.useHeightMap", true);
					}

					shader.setInt(("material." + name + number).c_str(), i + offset);
					glBindTexture(GL_TEXTURE_2D, textures[i]->id);
				}
			}
			else {
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
			}

			glActiveTexture(GL_TEXTURE0);
		}
		else {
			shader.setBool("material.useAlbedoMap", false);
			shader.setBool("material.useNormalMap", false);
			shader.setBool("material.useMetallicMap", false);
			shader.setBool("material.useRoughnessMap", false);
			shader.setBool("material.useAoMap", false);

			unsigned int albedoNr = 1;
			unsigned int normalNr = 1;
			unsigned int metallicNr = 1;
			unsigned int roughnessNr = 1;
			unsigned int aoNr = 1;

			std::string number;
			std::string name;
			for (unsigned int i = 0; i < textures.size(); i++) {
				glActiveTexture(GL_TEXTURE0 + i + offset);
				name = ConvertTextureTypeToString(textures[i]->type);
				if (name == ConvertTextureTypeToString(TEXTURE_ALBEDO)) {
					number = std::to_string(albedoNr++);
					shader.setBool("material.useAlbedoMap", true);
				}
				else if (name == ConvertTextureTypeToString(TEXTURE_NORMAL)) {
					number = std::to_string(normalNr++);
					shader.setBool("material.useNormalMap", true);
				}
				else if (name == ConvertTextureTypeToString(TEXTURE_METAL)) {
					number = std::to_string(metallicNr++);
					shader.setBool("material.useMetallicMap", true);
				}
				else if (name == ConvertTextureTypeToString(TEXTURE_ROUGHNESS)) {
					number = std::to_string(roughnessNr++);
					shader.setBool("material.useRoughnessMap", true);
				}
				else if (name == ConvertTextureTypeToString(TEXTURE_AO)) {
					number = std::to_string(aoNr++);
					shader.setBool("material.useAoMap", true);
				}

				shader.setInt(("material." + name + number).c_str(), i + offset);
				glBindTexture(GL_TEXTURE_2D, textures[i]->id);
			}
			glActiveTexture(GL_TEXTURE0);
		}

		// draw
		glBindVertexArray(VAO);
		glDrawElements(drawPrimitive, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
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
		//glEnableVertexAttribArray(5);
		//glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));

		// weights
		//glEnableVertexAttribArray(6);
		//glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));
		//glBindVertexArray(0);
	}
}