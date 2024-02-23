#include "Model.h"
#include "ResourceManager.h"
namespace Engine {
	Model::Model(PremadeModel modelType)
	{
		if (modelType == MODEL_PLANE) {
			meshes.push_back(*ResourceManager::GetInstance()->DefaultPlane());
		}
		else if (modelType == MODEL_CUBE) {
			meshes.push_back(*ResourceManager::GetInstance()->DefaultCube());
		}
		else if (modelType == MODEL_SPHERE) {
			meshes.push_back(*ResourceManager::GetInstance()->DefaultSphere());
		}
	}

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

	void Model::ApplyMaterialToAllMesh(Material* material)
	{
		for (Mesh& m : meshes) {
			m.ApplyMaterial(material);
		}
	}

	void Model::ApplyMaterialToMeshAtIndex(Material* material, int index)
	{
		if (index < meshes.size()) {
			meshes[index].ApplyMaterial(material);
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
		std::vector<Texture*> textures;
		Material* meshMaterial = new Material();

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

				float shine;
				if (AI_SUCCESS != aiGetMaterialFloat(material, AI_MATKEY_SHININESS, &shine)) {
					// default
					shine = 15.0f;
				}

				aiColor4D baseColour;
				if (AI_SUCCESS != aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &baseColour)) {
					// default
					baseColour = aiColor4D(1.0f, 1.0f, 1.0f, 1.0f);
				}
				glm::vec3 diffuseColour = glm::vec3(baseColour.r, baseColour.b, baseColour.g);

				aiColor4D specColour;
				if (AI_SUCCESS != aiGetMaterialColor(material, AI_MATKEY_COLOR_SPECULAR, &specColour)) {
					// default
					specColour = aiColor4D(1.0f, 1.0f, 1.0f, 1.0f);
				}
				glm::vec3 specularColour = glm::vec3(specColour.r, specColour.b, specColour.g);

				meshMaterial->diffuse = diffuseColour;
				meshMaterial->specular = specularColour;
				meshMaterial->shininess = shine;
				meshMaterial->height_scale = 10.0f;

				std::vector<Texture*> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, TEXTURE_DIFFUSE);
				meshMaterial->diffuseMaps = diffuseMaps;
				//textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

				std::vector<Texture*> specularMaps = LoadMaterialTextures(material, aiTextureType_SPECULAR, TEXTURE_SPECULAR);
				meshMaterial->specularMaps = specularMaps;
				//textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

				std::vector<Texture*> normalMaps = LoadMaterialTextures(material, aiTextureType_HEIGHT, TEXTURE_NORMAL);
				meshMaterial->normalMaps = normalMaps;
				//textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

				std::vector<Texture*> heightMaps = LoadMaterialTextures(material, aiTextureType_AMBIENT, TEXTURE_HEIGHT);
				meshMaterial->heightMaps = heightMaps;
				//textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

				return Mesh(vertices, indices, meshMaterial, pbr);
			}
			else {
				std::vector<Texture*> albedoMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, TEXTURE_ALBEDO);
				textures.insert(textures.end(), albedoMaps.begin(), albedoMaps.end());

				std::vector<Texture*> normalMaps = LoadMaterialTextures(material, aiTextureType_HEIGHT, TEXTURE_NORMAL);
				textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

				std::vector<Texture*> metallicMaps = LoadMaterialTextures(material, aiTextureType_SHININESS, TEXTURE_METAL);
				textures.insert(textures.end(), metallicMaps.begin(), metallicMaps.end());

				std::vector<Texture*> roughnessMaps = LoadMaterialTextures(material, aiTextureType_SPECULAR, TEXTURE_ROUGHNESS);
				textures.insert(textures.end(), roughnessMaps.begin(), roughnessMaps.end());

				std::vector<Texture*> aoMaps = LoadMaterialTextures(material, aiTextureType_AMBIENT, TEXTURE_AO);
				textures.insert(textures.end(), aoMaps.begin(), aoMaps.end());

				return Mesh(vertices, indices, textures, pbr);
			}
		}
	}

	std::vector<Texture*> Model::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, TextureTypes name)
	{
		std::vector<Texture*> textures;
		bool skip;
		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
			aiString str;
			mat->GetTexture(type, i, &str);

			textures.push_back(ResourceManager::GetInstance()->LoadTexture(directory + '/' + str.C_Str(), name));
		}

		return textures;
	}
}