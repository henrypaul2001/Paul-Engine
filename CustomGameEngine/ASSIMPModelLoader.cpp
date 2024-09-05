#include "ASSIMPModelLoader.h"
namespace Engine {
	const std::vector<MeshData*> ASSIMPModelLoader::LoadMeshData(const std::string& filepath, unsigned assimpPostProcess, bool persistentResources)
	{
		std::vector<MeshData*> meshList;
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(filepath, assimpPostProcess);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
			return meshList;
		}

		meshList = ProcessNode(filepath, scene->mRootNode, scene);

		/*
		if (hasBones) {
			ProcessEmptyBones(scene->mRootNode);
		}
		*/

		//CollectMeshMaterials();

		return meshList;
	}

	std::vector<MeshData*> ASSIMPModelLoader::ProcessNode(const std::string& filepath, aiNode* node, const aiScene* scene, bool persistentResources)
	{
		std::vector<MeshData*> meshList;
		meshList.reserve(node->mNumMeshes);
		for (unsigned int i = 0; i < node->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

			meshList.push_back(ProcessMesh(filepath, mesh, scene));

			//meshes.push_back(newMesh);
			/*
			if (!pbr) {
				if (newMesh->GetMaterial()->isTransparent) {
					containsTransparentMeshes = true;
				}
			}
			else {
				// PBR transparency check will go here in future
			}
			*/
		}

		for (unsigned int i = 0; i < node->mNumChildren; i++) {
			std::vector<MeshData*> childMeshes = ProcessNode(filepath, node->mChildren[i], scene);
			meshList.insert(meshList.end(), childMeshes.begin(), childMeshes.end());
		}

		return meshList;
	}

	MeshData* ASSIMPModelLoader::ProcessMesh(const std::string& filepath, aiMesh* mesh, const aiScene* scene, bool persistentResources)
	{
		ResourceManager* resources = ResourceManager::GetInstance();
		std::string meshName = mesh->mName.C_Str();
		std::string fileAndMeshName = filepath + "/" + meshName;
		std::cout << "ASSIMPModelLoader::Loading mesh " << meshName << std::endl;

		// Attempt to fetch from resource manager
		MeshData* meshData = resources->GetMeshData(fileAndMeshName);

		if (meshData == nullptr) {
			// Load mesh data
			std::vector<Vertex> vertices;
			std::vector<unsigned int> indices;

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

			// TODO: Create additional resource list in resource manager for animation skeletons <modelFilepath, AnimationSkeleton>
			/*
			// retrieve bones
			if (mesh->HasBones()) {
				ProcessBones(vertices, mesh, scene);
				std::vector<glm::mat4> meshBones = std::vector<glm::mat4>(mesh->mNumBones, glm::mat4(1.0f));
				skeleton.finalBoneMatrices.insert(skeleton.finalBoneMatrices.end(), meshBones.begin(), meshBones.end());
			}
			*/

			meshData = new MeshData(vertices, indices);
			ResourceManager::GetInstance()->AddMeshData(fileAndMeshName, meshData, persistentResources);
		}

		// Load materials
		if (mesh->mMaterialIndex >= 0) {
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			const std::string materialName = material->GetName().C_Str();
			const std::string fileAndMaterialName = filepath + "/" + materialName;

			// Attempt to fetch from resource manager
			AbstractMaterial* meshMaterial = resources->GetMaterial(fileAndMaterialName);

			if (meshMaterial == nullptr) {
				std::cout << "ASSIMPModelLoader::Loading material " << materialName << std::endl;

				if (!loadMaterialsAsPBR) {
					meshMaterial = LoadMaterialFromaiMat(material, scene, filepath);
				}
				else {
					meshMaterial = LoadPBRMaterialFromaiMat(material, scene, filepath);
				}
				resources->AddMaterial(fileAndMaterialName, meshMaterial, persistentResources);
			}
		}

		return meshData;
	}

	bool ASSIMPModelLoader::loadMaterialsAsPBR = false;
	Material* ASSIMPModelLoader::LoadMaterialFromaiMat(const aiMaterial* material, const aiScene* scene, const std::string& filepath)
	{
		std::unordered_map<TextureTypes, aiTextureType> textureTranslations = ResourceManager::GetInstance()->GetTextureTranslations();
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

		std::vector<Texture*> diffuseMaps = LoadMaterialTextures(filepath, material, textureTranslations[TEXTURE_DIFFUSE], TEXTURE_DIFFUSE, scene);
		std::vector<Texture*> specularMaps = LoadMaterialTextures(filepath, material, textureTranslations[TEXTURE_SPECULAR], TEXTURE_SPECULAR, scene);
		std::vector<Texture*> normalMaps = LoadMaterialTextures(filepath, material, textureTranslations[TEXTURE_NORMAL], TEXTURE_NORMAL, scene);
		std::vector<Texture*> heightMaps = LoadMaterialTextures(filepath, material, textureTranslations[TEXTURE_HEIGHT], TEXTURE_HEIGHT, scene);
		std::vector<Texture*> opacityMaps = LoadMaterialTextures(filepath, material, textureTranslations[TEXTURE_OPACITY], TEXTURE_OPACITY, scene);

		Material* standardMaterial = new Material(diffuseMaps, specularMaps, normalMaps, heightMaps, opacityMaps, shine);
		standardMaterial->height_scale = 10.0f; // this should be read from the material import instead
		standardMaterial->baseColour = diffuseColour;
		standardMaterial->specular = specularColour;

		return standardMaterial;
	}

	PBRMaterial* ASSIMPModelLoader::LoadPBRMaterialFromaiMat(const aiMaterial* material, const aiScene* scene, const std::string& filepath)
	{
		std::unordered_map<TextureTypes, aiTextureType> textureTranslations = ResourceManager::GetInstance()->GetTextureTranslations();
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

		std::vector<Texture*> albedoMaps = LoadMaterialTextures(filepath, material, textureTranslations[TEXTURE_ALBEDO], TEXTURE_ALBEDO, scene);
		std::vector<Texture*> normalMaps = LoadMaterialTextures(filepath, material, textureTranslations[TEXTURE_NORMAL], TEXTURE_NORMAL, scene);
		std::vector<Texture*> metallicMaps = LoadMaterialTextures(filepath, material, textureTranslations[TEXTURE_METALLIC], TEXTURE_METALLIC, scene);
		std::vector<Texture*> roughnessMaps = LoadMaterialTextures(filepath, material, textureTranslations[TEXTURE_ROUGHNESS], TEXTURE_ROUGHNESS, scene);
		std::vector<Texture*> aoMaps = LoadMaterialTextures(filepath, material, textureTranslations[TEXTURE_AO], TEXTURE_AO, scene);
		std::vector<Texture*> heightMaps = LoadMaterialTextures(filepath, material, textureTranslations[TEXTURE_HEIGHT], TEXTURE_HEIGHT, scene);
		std::vector<Texture*> opacityMaps = LoadMaterialTextures(filepath, material, textureTranslations[TEXTURE_OPACITY], TEXTURE_OPACITY, scene);

		PBRMaterial* pbrMaterial = new PBRMaterial(albedoMaps, normalMaps, metallicMaps, roughnessMaps, aoMaps, heightMaps, opacityMaps);
		pbrMaterial->baseColour = baseAlbedo;
		pbrMaterial->metallic = metal;
		pbrMaterial->roughness = rough;
		pbrMaterial->ao = 1.0f;
		pbrMaterial->height_scale = 10.0f; // this should be read from the material import instead
		return pbrMaterial;
	}

	std::vector<Texture*> ASSIMPModelLoader::LoadMaterialTextures(const std::string& filepath, const aiMaterial* mat, const aiTextureType type, const TextureTypes name, const aiScene* scene)
	{
		std::string directory = filepath.substr(0, filepath.find_last_of('/'));
		std::vector<Texture*> textures;
		bool skip;
		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
			aiString str;
			mat->GetTexture(type, i, &str);

			const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(str.C_Str());
			if (embeddedTexture) { textures.push_back(ResourceManager::GetInstance()->LoadTextureEmbedded(embeddedTexture, directory + '/' + str.C_Str(), name, (name == TEXTURE_ALBEDO || name == TEXTURE_DIFFUSE))); }
			else { textures.push_back(ResourceManager::GetInstance()->LoadTexture(directory + '/' + str.C_Str(), name, (name == TEXTURE_ALBEDO || name == TEXTURE_DIFFUSE))); }
		}

		return textures;
	}
}