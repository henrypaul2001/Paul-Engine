#include "ASSIMPModelLoader.h"
namespace Engine {
	const LoadMeshDataResult ASSIMPModelLoader::LoadMeshData(const std::string& filepath, unsigned assimpPostProcess, bool persistentResources)
	{
		ResourceManager* resources = ResourceManager::GetInstance();
		LoadMeshDataResult result;
		result.hasBones = false;
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(filepath, assimpPostProcess);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
			return result;
		}

		// Check resource manager for existing animation skeleton
		AnimationSkeleton* skeleton = resources->GetAnimationSkeleton(filepath);
		if (skeleton) {
			result.skeleton = skeleton;
			result.skeletonAlreadyLoaded = true;
		}
		else { 
			result.skeletonAlreadyLoaded = false;
			result.skeleton = new AnimationSkeleton();
			resources->AddAnimationSkeleton(filepath, result.skeleton);
		}

		ProcessNode(filepath, scene->mRootNode, scene, result);

		if (result.hasBones && !result.skeletonAlreadyLoaded) {
			ProcessEmptyBones(scene->mRootNode, result);
		}

		return result;
	}

	void ASSIMPModelLoader::ProcessNode(const std::string& filepath, aiNode* node, const aiScene* scene, LoadMeshDataResult& out_result, bool persistentResources)
	{
		out_result.meshResults.reserve(node->mNumMeshes);
		for (unsigned int i = 0; i < node->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

			ProcessMesh(filepath, mesh, scene, out_result);
		}

		for (unsigned int i = 0; i < node->mNumChildren; i++) {
			ProcessNode(filepath, node->mChildren[i], scene, out_result);
		}
	}

	void ASSIMPModelLoader::ProcessMesh(const std::string& filepath, aiMesh* mesh, const aiScene* scene, LoadMeshDataResult& out_result, bool persistentResources)
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

			// Retrieve bones
			if (mesh->HasBones() && !out_result.skeletonAlreadyLoaded) {
				ProcessBones(vertices, mesh, scene, out_result);
				std::vector<glm::mat4> meshBones = std::vector<glm::mat4>(mesh->mNumBones, glm::mat4(1.0f));
				out_result.skeleton->finalBoneMatrices.insert(out_result.skeleton->finalBoneMatrices.end(), meshBones.begin(), meshBones.end());
			}
			meshData = new MeshData(vertices, indices);
			resources->AddMeshData(fileAndMeshName, meshData, persistentResources);
		}

		ProcessMeshResult meshResult;

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

			meshResult.meshMaterials.push_back(meshMaterial);
		}
		meshResult.meshData = meshData;

		out_result.meshResults.push_back(meshResult);
	}

	void ASSIMPModelLoader::ProcessBones(std::vector<Vertex>& vertices, const aiMesh* mesh, const aiScene* scene, LoadMeshDataResult& out_result)
	{
		for (unsigned int i = 0; i < mesh->mNumBones; i++) {
			out_result.hasBones = true;
			AnimationSkeleton& skeleton = *out_result.skeleton;

			// Get bone ID
			// -----------
			int boneID = -1;
			std::string boneName = mesh->mBones[i]->mName.C_Str();

			if (skeleton.bones.find(boneName) == skeleton.bones.end()) {
				// Create new bone
				AnimationBone newBone;
				boneID = skeleton.bones.size();
				newBone.boneID = boneID;
				newBone.offsetMatrix = ConvertMatrixToGLMFormat(mesh->mBones[i]->mOffsetMatrix);
				newBone.nodeTransform = ConvertMatrixToGLMFormat(mesh->mBones[i]->mNode->mTransformation);
				newBone.name = boneName;

				// Get child bone names
				for (unsigned int j = 0; j < mesh->mBones[i]->mNode->mNumChildren; j++) {
					std::string childName = mesh->mBones[i]->mNode->mChildren[j]->mName.C_Str();
					newBone.childNodeNames.push_back(childName);
				}

				skeleton.bones[boneName] = newBone;

				if (skeleton.bones.find(mesh->mBones[i]->mNode->mParent->mName.C_Str()) == skeleton.bones.end()) {
					// Parent node name is not in bones list, this bone must be the root bone
					skeleton.rootBone = &skeleton.bones[boneName];
					skeleton.originTransform = ConvertMatrixToGLMFormat(mesh->mBones[i]->mNode->mParent->mTransformation);
				}
			}
			else {
				// Bone already exists
				boneID = skeleton.bones[boneName].boneID;
			}

			// Update vertices with IDs and weights
			for (unsigned int j = 0; j < mesh->mBones[i]->mNumWeights; j++) {
				const aiVertexWeight& aiWeight = mesh->mBones[i]->mWeights[j];
				int vertexID = aiWeight.mVertexId;
				assert(vertexID < vertices.size());
				vertices[vertexID].AddBoneData(boneID, aiWeight.mWeight);
			}
		}
	}

	bool ASSIMPModelLoader::ProcessEmptyBones(aiNode* node, LoadMeshDataResult& out_result)
	{
		if (node) {
			std::string nodeName = node->mName.C_Str();

			if (out_result.skeleton->bones.find(nodeName) == out_result.skeleton->bones.end()) {

				bool isInBoneBranch = false;
				for (int i = 0; i < node->mNumChildren; i++) {
					aiNode* child = node->mChildren[i];

					if (ProcessEmptyBones(child, out_result)) {
						isInBoneBranch = true;
					}
				}

				if (isInBoneBranch) {
					AnimationBone newBone;
					newBone.boneID = -1;
					newBone.nodeTransform = ConvertMatrixToGLMFormat(node->mTransformation);
					newBone.name = nodeName;

					for (int i = 0; i < node->mNumChildren; i++) {
						newBone.childNodeNames.push_back(node->mChildren[i]->mName.C_Str());
					}

					out_result.skeleton->emptyBones[nodeName] = newBone;
					out_result.skeleton->rootBone = &out_result.skeleton->emptyBones[nodeName];
				}
				return isInBoneBranch;
			}
			else {
				// Bone already attached to mesh, no need to process
				// Tell parent that this branch contains bones, thus a root node needs to be created above it. Return true
				for (int i = 0; i < node->mNumChildren; i++) {
					aiNode* child = node->mChildren[i];

					ProcessEmptyBones(child, out_result);
				}
				return true;
			}
		}
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