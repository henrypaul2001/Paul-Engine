#include "Model.h"
#include "ResourceManager.h"
namespace Engine {
	Model::Model(PremadeModel modelType, bool pbr)
	{
		if (modelType == MODEL_PLANE) {
			meshes.push_back(new Mesh(ResourceManager::GetInstance()->DefaultPlane()));
		}
		else if (modelType == MODEL_CUBE) {
			meshes.push_back(new Mesh(ResourceManager::GetInstance()->DefaultCube()));
		}
		else if (modelType == MODEL_SPHERE) {
			meshes.push_back(new Mesh(ResourceManager::GetInstance()->DefaultSphere()));
		}
		containsTransparentMeshes = false;
		this->pbr = pbr;
	}

	Model::Model(const char* filepath)
	{
		pbr = false;
		LoadModel(filepath);
		containsTransparentMeshes = false;
	}

	Model::Model(const char* filepath, bool pbr)
	{
		this->pbr = pbr;
		LoadModel(filepath);
		containsTransparentMeshes = false;
	}

	Model::~Model()
	{

	}

	void Model::Draw(Shader& shader, int instanceNum)
	{
		for (unsigned int i = 0; i < meshes.size(); i++) {
			if (!pbr) {
				if (!meshes[i]->GetMaterial()->isTransparent) {
					meshes[i]->Draw(shader, pbr, instanceNum);
				}
			}
			else {
				if (!meshes[i]->GetPBRMaterial()->isTransparent) {
					meshes[i]->Draw(shader, pbr, instanceNum);
				}
			}
		}
	}

	void Model::DrawTransparentMeshes(Shader& shader, int instanceNum)
	{
		for (unsigned int i = 0; i < meshes.size(); i++) {
			if (!pbr) {
				if (meshes[i]->GetMaterial()->isTransparent) {
					meshes[i]->Draw(shader, pbr, instanceNum);
				}
			}
			else {
				if (meshes[i]->GetPBRMaterial()->isTransparent) {
					meshes[i]->Draw(shader, pbr, instanceNum);
				}
			}
		}
	}

	void Model::ApplyMaterialToAllMesh(Material* material)
	{
		containsTransparentMeshes = material->isTransparent;
		for (Mesh* m : meshes) {
			m->ApplyMaterial(material);
		}
	}

	void Model::ApplyMaterialToMeshAtIndex(Material* material, int index)
	{
		containsTransparentMeshes = material->isTransparent;
		if (index < meshes.size()) {
			meshes[index]->ApplyMaterial(material);
		}
	}

	void Model::ApplyMaterialToAllMesh(PBRMaterial* pbrMaterial)
	{
		containsTransparentMeshes = pbrMaterial->isTransparent;
		for (Mesh* m : meshes) {
			m->ApplyMaterial(pbrMaterial);
		}
	}

	void Model::ApplyMaterialToMeshAtIndex(PBRMaterial* pbrMaterial, int index)
	{
		containsTransparentMeshes = pbrMaterial->isTransparent;
		if (index < meshes.size()) {
			meshes[index]->ApplyMaterial(pbrMaterial);
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
			
			Mesh* newMesh = ProcessMesh(mesh, scene);
			meshes.push_back(newMesh);
			if (!pbr) {
				if (newMesh->GetMaterial()->isTransparent) {
					containsTransparentMeshes = true;
				}
			}
			else {
				// PBR transparency check will go here in future
			}
		}

		for (unsigned int i = 0; i < node->mNumChildren; i++) {
			ProcessNode(node->mChildren[i], scene);
		}
	}

	Mesh* Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
	{
		std::cout << "MODEL::Loading mesh " << mesh->mName.C_Str() << std::endl;
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<Texture*> textures;
		Material* meshMaterial = new Material();
		PBRMaterial* pbrMaterial = new PBRMaterial();

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
			std::cout << "MODEL::Loading material " << material->GetName().C_Str() << std::endl;

			if (!pbr) {
				delete pbrMaterial;
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
				meshMaterial->height_scale = 10.0f; // this should be read from the material import instead

				std::vector<Texture*> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, TEXTURE_DIFFUSE);
				meshMaterial->diffuseMaps = diffuseMaps;

				std::vector<Texture*> specularMaps = LoadMaterialTextures(material, aiTextureType_SPECULAR, TEXTURE_SPECULAR);
				meshMaterial->specularMaps = specularMaps;

				std::vector<Texture*> normalMaps = LoadMaterialTextures(material, aiTextureType_HEIGHT, TEXTURE_NORMAL);
				meshMaterial->normalMaps = normalMaps;

				std::vector<Texture*> heightMaps = LoadMaterialTextures(material, aiTextureType_AMBIENT, TEXTURE_HEIGHT);
				meshMaterial->heightMaps = heightMaps;

				std::vector<Texture*> opacityMaps = LoadMaterialTextures(material, aiTextureType_OPACITY, TEXTURE_OPACITY);
				meshMaterial->opacityMaps = opacityMaps;

				if (opacityMaps.size() > 0) {
					meshMaterial->isTransparent = true;
				}

				return new Mesh(vertices, indices, meshMaterial);
			}
			else {
				delete meshMaterial;

				aiColor4D baseColour;
				if (AI_SUCCESS != aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &baseColour)) {
					// default
					baseColour = aiColor4D(1.0f, 1.0f, 1.0f, 1.0f);
				}
				glm::vec3 baseAlbedo = glm::vec3(baseColour.r, baseColour.b, baseColour.g);

				float metal;
				if (AI_SUCCESS != aiGetMaterialFloat(material, AI_MATKEY_METALLIC_FACTOR, &metal)) {
					// default
					metal = 1.0f;
				}

				float rough;
				if (AI_SUCCESS != aiGetMaterialFloat(material, AI_MATKEY_ROUGHNESS_FACTOR, &rough)) {
					// default
					rough = 1.0f;
				}

				pbrMaterial->albedo = baseAlbedo;
				pbrMaterial->metallic = metal;
				pbrMaterial->roughness = rough;
				pbrMaterial->ao = 1.0f;
				pbrMaterial->height_scale = 10.0f; // this should be read from the material import instead

				std::vector<Texture*> albedoMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, TEXTURE_ALBEDO);
				pbrMaterial->albedoMaps = albedoMaps;

				std::vector<Texture*> normalMaps = LoadMaterialTextures(material, aiTextureType_HEIGHT, TEXTURE_NORMAL);
				pbrMaterial->normalMaps;

				std::vector<Texture*> metallicMaps = LoadMaterialTextures(material, aiTextureType_SHININESS, TEXTURE_METALLIC);
				pbrMaterial->metallicMaps = metallicMaps;

				std::vector<Texture*> roughnessMaps = LoadMaterialTextures(material, aiTextureType_SPECULAR, TEXTURE_ROUGHNESS);
				pbrMaterial->roughnessMaps = roughnessMaps;

				std::vector<Texture*> aoMaps = LoadMaterialTextures(material, aiTextureType_AMBIENT, TEXTURE_AO);
				pbrMaterial->aoMaps = aoMaps;

				std::vector<Texture*> heightMaps = LoadMaterialTextures(material, aiTextureType_AMBIENT, TEXTURE_HEIGHT);
				pbrMaterial->heightMaps = heightMaps;

				std::vector<Texture*> opacityMaps = LoadMaterialTextures(material, aiTextureType_OPACITY, TEXTURE_OPACITY);
				pbrMaterial->opacityMaps = opacityMaps;

				if (opacityMaps.size() > 0) {
					pbrMaterial->isTransparent = true;
				}

				return new Mesh(vertices, indices, pbrMaterial);
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

			bool srgb = false;
			if (name == TEXTURE_ALBEDO || name == TEXTURE_DIFFUSE) {
				srgb = true;
			}
			textures.push_back(ResourceManager::GetInstance()->LoadTexture(directory + '/' + str.C_Str(), name, srgb));
			//textures.push_back(ResourceManager::GetInstance()->LoadTexture(str.C_Str(), name));
		}

		return textures;
	}
}