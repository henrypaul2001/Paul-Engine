#include "ASSIMPModelLoader.h"
#include "ResourceManager.h"
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
		std::string directory = filepath.substr(0, filepath.find_last_of('/'));

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
		std::string meshName = mesh->mName.C_Str();
		std::string fileAndMeshName = filepath + "/" + meshName;
		std::cout << "ASSIMPModelLoader::Loading mesh " << meshName << std::endl;

		// Attempt to fetch from resource manager
		MeshData* meshData = ResourceManager::GetInstance()->GetMeshData(fileAndMeshName);

		if (meshData == nullptr) {
			// Load mesh data
			std::vector<Vertex> vertices;
			std::vector<unsigned int> indices;
			//std::vector<Texture*> textures;
			//Material* meshMaterial = new Material();
			//PBRMaterial* pbrMaterial = new PBRMaterial();
			//std::unordered_map<TextureTypes, aiTextureType> textureTranslations = ResourceManager::GetInstance()->GetTextureTranslations();

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

		return meshData;
		
		// TODO: Create additional resource list in resource manager for materials <modelFilepath/materialName, Material>
		/*
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
				meshMaterial->useDiffuseAlphaAsOpacity = true;

				std::vector<Texture*> diffuseMaps = LoadMaterialTextures(material, textureTranslations[TEXTURE_DIFFUSE], TEXTURE_DIFFUSE, scene);
				meshMaterial->diffuseMaps = diffuseMaps;

				std::vector<Texture*> specularMaps = LoadMaterialTextures(material, textureTranslations[TEXTURE_SPECULAR], TEXTURE_SPECULAR, scene);
				meshMaterial->specularMaps = specularMaps;

				std::vector<Texture*> normalMaps = LoadMaterialTextures(material, textureTranslations[TEXTURE_NORMAL], TEXTURE_NORMAL, scene);
				meshMaterial->normalMaps = normalMaps;

				std::vector<Texture*> heightMaps = LoadMaterialTextures(material, textureTranslations[TEXTURE_HEIGHT], TEXTURE_HEIGHT, scene);
				meshMaterial->heightMaps = heightMaps;

				std::vector<Texture*> opacityMaps = LoadMaterialTextures(material, textureTranslations[TEXTURE_OPACITY], TEXTURE_OPACITY, scene);
				meshMaterial->opacityMaps = opacityMaps;

				if (opacityMaps.size() > 0) {
					meshMaterial->isTransparent = true;
					meshMaterial->useDiffuseAlphaAsOpacity = false;
				}

				// retrieve bones
				if (mesh->HasBones()) {
					ProcessBones(vertices, mesh, scene);
					std::vector<glm::mat4> meshBones = std::vector<glm::mat4>(mesh->mNumBones, glm::mat4(1.0f));
					skeleton.finalBoneMatrices.insert(skeleton.finalBoneMatrices.end(), meshBones.begin(), meshBones.end());
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
				pbrMaterial->useDiffuseAlphaAsOpacity = true;

				std::vector<Texture*> albedoMaps = LoadMaterialTextures(material, textureTranslations[TEXTURE_ALBEDO], TEXTURE_ALBEDO, scene);
				pbrMaterial->albedoMaps = albedoMaps;

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
				pbrMaterial->opacityMaps = opacityMaps;

				if (opacityMaps.size() > 0) {
					pbrMaterial->isTransparent = true;
					pbrMaterial->useDiffuseAlphaAsOpacity = false;
				}

				// retrieve bones
				if (mesh->HasBones()) {
					ProcessBones(vertices, mesh, scene);
					std::vector<glm::mat4> meshBones = std::vector<glm::mat4>(mesh->mNumBones, glm::mat4(1.0f));
					skeleton.finalBoneMatrices.insert(skeleton.finalBoneMatrices.end(), meshBones.begin(), meshBones.end());
				}

				return new Mesh(vertices, indices, pbrMaterial);
			}
		}
		*/
		
	}
}