#include "Model.h"
namespace Engine {
	Model::Model(const char* filepath)
	{
		LoadModel(filepath);
		pbr = false;
	}

	Model::Model(const char* filepath, bool pbr)
	{
		LoadModel(filepath);
		this->pbr = pbr;
	}

	Model::~Model()
	{

	}

	void Model::Draw(Shader& shader)
	{
		for (unsigned int i = 0; i < meshes.size(); i++) {
			meshes[i].Draw(shader);
		}
	}

	void Model::LoadModel(std::string filepath)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(filepath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
			return;
		}
		directory = filepath.substr(0, filepath.find_last_of('/'));

		ProcessNode(scene->mRootNode, scene);
	}

	void Model::ProcessNode(aiNode* node, const aiScene* scene)
	{
		for (unsigned int i = 0; i < node->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.push_back(ProcessMesh(mesh, scene));
		}

		for (unsigned int i = 0; i < node->mNumChildren; i++) {
			ProcessNode(node->mChildren[i], scene);
		}
	}

	Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<Texture> textures;

		// retrieve vertices
		Vertex vertex;
		glm::vec2 vec;
		for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
			// retrieve positions
			vertex.Position.x = mesh->mVertices[i].x;
			vertex.Position.y = mesh->mVertices[i].y;
			vertex.Position.z = mesh->mVertices[i].z;

			// retrieve normals
			vertex.Normal.x = mesh->mNormals[i].x;
			vertex.Normal.y = mesh->mNormals[i].y;
			vertex.Normal.z = mesh->mNormals[i].z;

			// retrieve texcoords
			if (mesh->mTextureCoords[0]) {
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.TexCoords = vec;
				// tangent
				vertex.Tangent.x = mesh->mTangents[i].x;
				vertex.Tangent.y = mesh->mTangents[i].y;
				vertex.Tangent.z = mesh->mTangents[i].z;
				// bitangent
				vertex.Bitangent.x = mesh->mBitangents[i].x;
				vertex.Bitangent.y = mesh->mBitangents[i].y;
				vertex.Bitangent.z = mesh->mBitangents[i].z;
			}
			else {
				vec.x = 0.0f;
				vec.y = 0.0f;
				vertex.TexCoords = vec;
			}

			vertices.push_back(vertex);
		}

		// retrieve indices
		for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++) {
				indices.push_back(face.mIndices[j]);
			}
		}

		// retrieve materials
		if (mesh->mMaterialIndex >= 0) {
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

			if (!pbr) {
				std::vector<Texture> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, TEXTURE_DIFFUSE);
				textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

				std::vector<Texture> specularMaps = LoadMaterialTextures(material, aiTextureType_SPECULAR, TEXTURE_SPECULAR);
				textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

				std::vector<Texture> normalMaps = LoadMaterialTextures(material, aiTextureType_HEIGHT, TEXTURE_NORMAL);
				textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

				std::vector<Texture> heightMaps = LoadMaterialTextures(material, aiTextureType_AMBIENT, TEXTURE_HEIGHT);
				textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
			}
			else {
				std::vector<Texture> albedoMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, TEXTURE_ALBEDO);
				textures.insert(textures.end(), albedoMaps.begin(), albedoMaps.end());

				std::vector<Texture> normalMaps = LoadMaterialTextures(material, aiTextureType_HEIGHT, TEXTURE_NORMAL);
				textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

				std::vector<Texture> metallicMaps = LoadMaterialTextures(material, aiTextureType_SHININESS, TEXTURE_METAL);
				textures.insert(textures.end(), metallicMaps.begin(), metallicMaps.end());

				std::vector<Texture> roughnessMaps = LoadMaterialTextures(material, aiTextureType_SPECULAR, TEXTURE_ROUGHNESS);
				textures.insert(textures.end(), roughnessMaps.begin(), roughnessMaps.end());

				std::vector<Texture> aoMaps = LoadMaterialTextures(material, aiTextureType_AMBIENT, TEXTURE_AO);
				textures.insert(textures.end(), aoMaps.begin(), aoMaps.end());
			}
		}

		return Mesh(vertices, indices, textures, pbr);
	}

	std::vector<Texture> Model::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, TextureTypes name)
	{
		std::vector<Texture> textures;
		bool skip;
		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
			aiString str;
			mat->GetTexture(type, i, &str);

			skip = false;
			for (unsigned int j = 0; j < textures_loaded.size(); j++) {
				if (std::strcmp(textures_loaded[j].filepath.data(), str.C_Str()) == 0) {
					textures.push_back(textures_loaded[j]);
					skip = true;
					break;
				}
			}

			if (!skip) {
				Texture texture;
				texture.id = TextureFromFile(str.C_Str(), directory);
				texture.type = name;
				texture.filepath = str.C_Str();
				textures.push_back(texture);
				textures_loaded.push_back(texture);
			}
		}

		return textures;
	}

	unsigned int Model::TextureFromFile(const char* path, const std::string& directory)
	{
		std::string filename = std::string(path);
		filename = directory + '/' + filename;

		unsigned int textureID;
		glGenTextures(1, &textureID);

		int width, height, nrComponents;
		unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
		if (data) {
			GLenum format;
			if (nrComponents == 1) {
				format = GL_RED;
			}
			else if (nrComponents == 3) {
				format = GL_RGB;
			}
			else if (nrComponents == 4) {
				format = GL_RGBA;
			}

			glBindTexture(GL_TEXTURE_2D, textureID);
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			stbi_image_free(data);
		}
		else {
			std::cout << "ERROR::ASSIMP::TEXTURELOAD::Texture failed to load at path: " << path << std::endl;
			stbi_image_free(data);
		}
		
		return textureID;
	}
}