#include "Model.h"
#include "ResourceManager.h"
#include "ComponentGeometry.h"
namespace Engine {
	Model::Model(const Model& old_model)
	{
		for (Mesh* oldMesh : old_model.meshes) {
			this->meshes.push_back(new Mesh(*oldMesh));
		}
		this->pbr = old_model.pbr;
		this->hasBones = old_model.hasBones;
		this->containsTransparentMeshes = old_model.containsTransparentMeshes;
		this->directory = old_model.directory;
		this->skeleton = old_model.skeleton;
		this->hasBones = old_model.hasBones;

		for (unsigned int i = 0; i < meshes.size(); i++) {
			Mesh* m = meshes[i];
			m->SetOwner(this);
			m->SetLocalMeshID(i);
		}
	}

	Model::Model(const std::vector<Mesh*>& meshes, bool pbr)
	{
		this->meshes = meshes;
		this->hasBones = false;
		this->pbr = pbr;

		for (unsigned int i = 0; i < meshes.size(); i++) {
			Mesh* m = meshes[i];
			m->SetOwner(this);
			m->SetLocalMeshID(i);
		}
	}

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
		hasBones = false;
		this->pbr = pbr;

		for (unsigned int i = 0; i < meshes.size(); i++) {
			Mesh* m = meshes[i];
			m->SetOwner(this);
			m->SetLocalMeshID(i);
		}
	}
	/*
	Model::Model(const char* filepath, unsigned int assimpPostProcess)
	{
		pbr = false;
		containsTransparentMeshes = false;
		hasBones = false;
		LoadModel(filepath, assimpPostProcess);

		for (unsigned int i = 0; i < meshes.size(); i++) {
			Mesh* m = meshes[i];
			m->SetOwner(this);
			m->SetLocalMeshID(i);
		}
	}

	Model::Model(const char* filepath, unsigned assimpPostProcess, bool pbr)
	{
		this->pbr = pbr;
		containsTransparentMeshes = false;
		hasBones = false;
		LoadModel(filepath, assimpPostProcess);

		for (unsigned int i = 0; i < meshes.size(); i++) {
			Mesh* m = meshes[i];
			m->SetOwner(this);
			m->SetLocalMeshID(i);
		}
	}
	*/

	Model::~Model()
	{
		for (Mesh* m : meshes) {
			//glDeleteVertexArrays(1, &m->VAO);
			delete m;
		}
	}

	void Model::Draw(Shader& shader, int instanceNum, const std::vector<unsigned int> instanceVAOs)
	{
		SCOPE_TIMER("Model::Draw");
		for (unsigned int i = 0; i < meshes.size(); i++) {
			if (instanceNum > 0) {
				meshes[i]->Draw(shader, pbr, instanceNum, instanceVAOs[i]);
			}
			else {
				meshes[i]->Draw(shader, pbr, instanceNum);
			}
			/*
			if (!meshes[i]->GetMaterial()->isTransparent) {
				meshes[i]->Draw(shader, pbr, instanceNum);
			}
			*/
		}
	}

	void Model::DrawTransparentMeshes(Shader& shader, int instanceNum, const std::vector<unsigned int> instanceVAOs)
	{
		SCOPE_TIMER("Model::DrawTransparentMeshes");
		for (unsigned int i = 0; i < meshes.size(); i++) {
			if (meshes[i]->GetMaterial()->GetIsTransparent()) {
				if (instanceNum > 0) {
					meshes[i]->Draw(shader, pbr, instanceNum, instanceVAOs[i]);
				}
				else {
					meshes[i]->Draw(shader, pbr, instanceNum);
				}
			}
		}
	}

	void Model::ApplyMaterialsToAllMesh(const std::vector<AbstractMaterial*>& materials)
	{
		pbr = materials[0]->IsPBR();
		for (AbstractMaterial* m : materials) {
			if (m->GetIsTransparent()) {
				containsTransparentMeshes = true;
			}
		}

		for (Mesh* m : meshes) {
			m->SetMaterials(materials);
		}
	}

	void Model::ApplyMaterialsToMeshAtIndex(const std::vector<AbstractMaterial*>& materials, const unsigned int index)
	{
		pbr = materials[0]->IsPBR();
		for (AbstractMaterial* m : materials) {
			if (m->GetIsTransparent()) {
				containsTransparentMeshes = true;
			}
		}

		if (index < meshes.size()) {
			meshes[index]->SetMaterials(materials);
		}
	}

	/*
	void Model::LoadModel(std::string filepath, unsigned int assimpPostProcess)
	{
		std::cout << "Model::LoadModel deprecated" << std::endl;
		return;
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(filepath, assimpPostProcess);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
			return;
		}
		directory = filepath.substr(0, filepath.find_last_of('/'));

		ProcessNode(scene->mRootNode, scene);

		if (hasBones) {
			ProcessEmptyBones(scene->mRootNode);
		}

		//CollectMeshMaterials();
	}

	void Model::ProcessNode(aiNode* node, const aiScene* scene)
	{
		for (unsigned int i = 0; i < node->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			
			Mesh* newMesh = ProcessMesh(mesh, scene);
			meshes.push_back(newMesh);
			if (!pbr) {
				if (newMesh->GetMaterial()->GetIsTransparent()) {
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
		
		std::cout << "Model::ProcessMesh deprecated" << std::endl;
		return nullptr;

		std::cout << "MODEL::Loading mesh " << mesh->mName.C_Str() << std::endl;
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<Texture*> textures;
		Material* meshMaterial = new Material(glm::vec3(1.0f));
		PBRMaterial* pbrMaterial = new PBRMaterial(glm::vec3(1.0f));
		std::unordered_map<TextureTypes, aiTextureType> textureTranslations = ResourceManager::GetInstance()->GetTextureTranslations();

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

				meshMaterial->baseColour = diffuseColour;
				meshMaterial->specular = specularColour;
				meshMaterial->shininess = shine;
				meshMaterial->height_scale = 10.0f; // this should be read from the material import instead
				meshMaterial->SetUseColourMapAsAlpha(true);

				std::vector<Texture*> diffuseMaps = LoadMaterialTextures(material, textureTranslations[TEXTURE_DIFFUSE], TEXTURE_DIFFUSE, scene);
				meshMaterial->baseColourMaps = diffuseMaps;

				std::vector<Texture*> specularMaps = LoadMaterialTextures(material, textureTranslations[TEXTURE_SPECULAR], TEXTURE_SPECULAR, scene);
				meshMaterial->specularMaps = specularMaps;

				std::vector<Texture*> normalMaps = LoadMaterialTextures(material, textureTranslations[TEXTURE_NORMAL], TEXTURE_NORMAL, scene);
				meshMaterial->normalMaps = normalMaps;

				std::vector<Texture*> heightMaps = LoadMaterialTextures(material, textureTranslations[TEXTURE_HEIGHT], TEXTURE_HEIGHT, scene);
				meshMaterial->heightMaps = heightMaps;

				std::vector<Texture*> opacityMaps = LoadMaterialTextures(material, textureTranslations[TEXTURE_OPACITY], TEXTURE_OPACITY, scene);

				for (Texture* t : opacityMaps) {
					meshMaterial->PushOpacityMap(t);
				}

				// retrieve bones
				if (mesh->HasBones()) {
					ProcessBones(vertices, mesh, scene);
					std::vector<glm::mat4> meshBones = std::vector<glm::mat4>(mesh->mNumBones, glm::mat4(1.0f));
					skeleton.finalBoneMatrices.insert(skeleton.finalBoneMatrices.end(), meshBones.begin(), meshBones.end());
				}

				//return new Mesh(vertices, indices, meshMaterial);
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

				pbrMaterial->baseColour = baseAlbedo;
				pbrMaterial->metallic = metal;
				pbrMaterial->roughness = rough;
				pbrMaterial->ao = 1.0f;
				pbrMaterial->height_scale = 10.0f; // this should be read from the material import instead
				pbrMaterial->SetUseColourMapAsAlpha(true);

				std::vector<Texture*> albedoMaps = LoadMaterialTextures(material, textureTranslations[TEXTURE_ALBEDO], TEXTURE_ALBEDO, scene);
				pbrMaterial->baseColourMaps = albedoMaps;

				std::vector<Texture*> normalMaps = LoadMaterialTextures(material, textureTranslations[TEXTURE_NORMAL], TEXTURE_NORMAL, scene);
				pbrMaterial->normalMaps = normalMaps;

				std::vector<Texture*> metallicMaps = LoadMaterialTextures(material, textureTranslations[TEXTURE_METALLIC], TEXTURE_METALLIC, scene);
				pbrMaterial->metallicMaps = metallicMaps;

				std::vector<Texture*> roughnessMaps = LoadMaterialTextures(material, textureTranslations[TEXTURE_ROUGHNESS], TEXTURE_ROUGHNESS, scene);
				pbrMaterial->roughnessMaps = roughnessMaps;

				std::vector<Texture*> aoMaps = LoadMaterialTextures(material, textureTranslations[TEXTURE_AO], TEXTURE_AO, scene);
				pbrMaterial->aoMaps = aoMaps;

				std::vector<Texture*> heightMaps = LoadMaterialTextures(material, textureTranslations[TEXTURE_HEIGHT], TEXTURE_HEIGHT, scene);
				pbrMaterial->heightMaps = heightMaps;

				std::vector<Texture*> opacityMaps = LoadMaterialTextures(material, textureTranslations[TEXTURE_OPACITY], TEXTURE_OPACITY, scene);

				for (Texture* t : opacityMaps) {
					pbrMaterial->PushOpacityMap(t);
				}

				// retrieve bones
				if (mesh->HasBones()) {
					ProcessBones(vertices, mesh, scene);
					std::vector<glm::mat4> meshBones = std::vector<glm::mat4>(mesh->mNumBones, glm::mat4(1.0f));
					skeleton.finalBoneMatrices.insert(skeleton.finalBoneMatrices.end(), meshBones.begin(), meshBones.end());
				}

				//return new Mesh(vertices, indices, pbrMaterial);
			}
		}
		
	}

	void Model::ProcessBones(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene)
	{
		
		for (unsigned int i = 0; i < mesh->mNumBones; i++) {
			hasBones = true;

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

	
	bool Model::ProcessEmptyBones(aiNode* node)
	{
		if (node) {
			std::string nodeName = node->mName.C_Str();

			if (skeleton.bones.find(nodeName) == skeleton.bones.end()) {

				bool isInBoneBranch = false;
				for (int i = 0; i < node->mNumChildren; i++) {
					aiNode* child = node->mChildren[i];

					if (ProcessEmptyBones(child)) {
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

					skeleton.emptyBones[nodeName] = newBone;
					skeleton.rootBone = &skeleton.emptyBones[nodeName];
				}
				return isInBoneBranch;
			}
			else {
				// Bone already attached to mesh, no need to process
				// Tell parent that this branch contains bones, thus a root node needs to be created above it. Return true
				for (int i = 0; i < node->mNumChildren; i++) {
					aiNode* child = node->mChildren[i];

					ProcessEmptyBones(child);
				}
				return true;
			}
		}
	}

	std::vector<Texture*> Model::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, TextureTypes name, const aiScene* scene)
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

			const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(str.C_Str());
			
			if (embeddedTexture) {
				textures.push_back(ResourceManager::GetInstance()->LoadTextureEmbedded(embeddedTexture, directory + '/' + str.C_Str(), name, srgb));
			}
			else {
				textures.push_back(ResourceManager::GetInstance()->LoadTexture(directory + '/' + str.C_Str(), name, srgb));
				//textures.push_back(ResourceManager::GetInstance()->LoadTexture(str.C_Str(), name));
			}
		}

		return textures;
	}
	*/
}