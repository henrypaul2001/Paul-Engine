#include "pepch.h"
#include "MeshImporter.h"
#include "MaterialImporter.h"
#include "PaulEngine/Project/Project.h"
#include "PaulEngine/Asset/AssetManager.h"
#include "PaulEngine/Renderer/Renderer.h"

#include "PaulEngine/Scene/Scene.h"
#include "PaulEngine/Scene/Components.h"
#include "PaulEngine/Scene/Entity.h"
#include "PaulEngine/Scene/Prefab.h"
#include "PaulEngine/Asset/SceneImporter.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/ext/matrix_float4x4.hpp>
#include <yaml-cpp/yaml.h>

namespace PaulEngine
{
	struct AssimpMeshResult
	{
		Ref<Mesh> LoadedMesh;
		unsigned int MaterialIndex;
	};

	struct AssimpModelResult
	{
		bool Success = false;
		std::vector<AssimpMeshResult> Meshes = {};
		//std::vector<AssimpNodeResult> Nodes = {};
	};

	struct AssimpNodeResult
	{
		std::string Name;
		glm::mat4 Transform = glm::mat4(1.0f);
		std::vector<unsigned int> MeshIndices; // indexed to aiScene mesh list
		std::vector<unsigned int> MaterialIndices; // element 0 in this list represents the material index of mesh 0 in MeshIndices

		std::vector<unsigned int> ChildrenIndices; // indexed to AssimpNodeTreeResult Nodes list
	};

	struct AssimpNodeTreeResult
	{
		std::vector<AssimpNodeResult> Nodes = {};
	};

	static glm::mat4 AssimpMat4ToGLMMat4(const aiMatrix4x4& from)
	{
		glm::mat4 to;
		//the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
		to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
		to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
		to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
		to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
		return to;
	}

	static unsigned int ReadAssimpNode(const aiScene* scene, const aiNode* node, AssimpNodeTreeResult& result)
	{
		AssimpNodeResult nodeResult;
		nodeResult.Name = node->mName.C_Str();
		nodeResult.Transform = AssimpMat4ToGLMMat4(node->mTransformation);

		unsigned int numMeshes = node->mNumMeshes;
		nodeResult.MeshIndices.reserve(numMeshes);
		for (unsigned int i = 0; i < numMeshes; i++)
		{
			nodeResult.MeshIndices.push_back(node->mMeshes[i]);
			
			const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			nodeResult.MaterialIndices.push_back(mesh->mMaterialIndex);
		}

		unsigned int numChildren = node->mNumChildren;
		for (unsigned int i = 0; i < numChildren; i++)
		{
			unsigned int childIndex = ReadAssimpNode(scene, node->mChildren[i], result);
			nodeResult.ChildrenIndices.push_back(childIndex);
		}

		result.Nodes.push_back(nodeResult);
		return result.Nodes.size() - 1;
	}

	static Entity CreateEntityFromNode(Ref<Scene>& scene, const AssimpNodeTreeResult& nodeTree, const unsigned int nodeIndex, const std::vector<AssetHandle>& meshHandles, const std::vector<AssetHandle>& materialHandles)
	{
		const AssimpNodeResult& node = nodeTree.Nodes[nodeIndex];

		Entity newEntity = scene->CreateEntity(node.Name);

		glm::vec3 pos;
		glm::vec3 rot;
		glm::vec3 scale;
		Maths::DecomposeTransform(node.Transform, pos, rot, scale);
		ComponentTransform& transform = newEntity.GetComponent<ComponentTransform>();
		transform.SetLocalPosition(pos);
		transform.SetLocalRotation(rot);
		transform.SetLocalScale(scale);

		if (!node.MeshIndices.empty())
		{
			ComponentMeshRenderer& meshComponent = newEntity.AddComponent<ComponentMeshRenderer>();
			meshComponent.MeshHandle = meshHandles[node.MeshIndices[0]];
			
			if (!node.MaterialIndices.empty())
			{
				ComponentMeshRenderer::SetMaterial(newEntity, materialHandles[node.MaterialIndices[0]]);
			}
		}

		unsigned int numChildren = node.ChildrenIndices.size();
		for (int i = 0; i < numChildren; i++)
		{
			Entity childEntity = CreateEntityFromNode(scene, nodeTree, node.ChildrenIndices[i], meshHandles, materialHandles);
			ComponentTransform::SetParent(childEntity, newEntity, false);
		}

		return newEntity;
	}

	static AssimpMeshResult ReadAssimpMesh(const aiMesh* mesh)
	{
		PE_PROFILE_FUNCTION();
		MeshSpecification spec;
		spec.Name = mesh->mName.C_Str();
		if (spec.Name.empty())
		{
			spec.Name = "Unnamed Mesh";
		}
		spec.Name.resize(25);
		spec.Name.resize(std::strlen(spec.Name.c_str()));
		spec.CalculateTangents = false;

		// Read vertices
		unsigned int numVertices = mesh->mNumVertices;
		std::vector<MeshVertex> vertices = std::vector<MeshVertex>(numVertices);
		for (unsigned int i = 0; i < numVertices; i++)
		{
			MeshVertex& vertex = vertices[i];

			const aiVector3D& aiPosition = mesh->mVertices[i];
			const aiVector3D& aiNormal = mesh->mNormals[i];
			const aiVector3D& aiUV = mesh->mTextureCoords[0][i];
			const aiVector3D& aiTangent = mesh->mTangents[i];
			const aiVector3D& aiBitangent = mesh->mBitangents[i];

			vertex.Position = glm::vec3(aiPosition.x, aiPosition.y, aiPosition.z);
			vertex.Normal = glm::vec3(aiNormal.x, aiNormal.y, aiNormal.z);
			vertex.TexCoords = glm::vec2(aiUV.x, aiUV.y);
			vertex.Tangent = glm::vec3(aiTangent.x, aiTangent.y, aiTangent.z);
			vertex.Bitangent = glm::vec3(aiBitangent.x, aiBitangent.y, aiBitangent.z);
		}

		// Read indices
		std::vector<uint32_t> indices;
		unsigned int numFaces = mesh->mNumFaces;
		indices.reserve(numFaces * 3);
		for (unsigned int i = 0; i < numFaces; i++)
		{
			const aiFace& face = mesh->mFaces[i];
			unsigned int numIndices = face.mNumIndices;
			for (unsigned int j = 0; j < numIndices; j++)
			{
				indices.push_back(face.mIndices[j]);
			}
		}

		// Create mesh (but don't register in asset manager yet)
		AssimpMeshResult result;
		result.LoadedMesh = CreateRef<Mesh>(spec, vertices, indices);
		result.MaterialIndex = mesh->mMaterialIndex;

		return result;
	}

	const aiScene* GetAssimpScene(Assimp::Importer& importer, const std::filesystem::path& filepath)
	{
		PE_PROFILE_FUNCTION();
		bool success = importer.ValidateFlags(
			aiProcess_GenNormals |
			aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_OptimizeMeshes |
			aiProcess_GenUVCoords |
			aiProcess_FixInfacingNormals |
			aiProcess_RemoveRedundantMaterials
		);
		const aiScene* scene = importer.ReadFile(filepath.string(),
			aiProcess_GenNormals |
			aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_OptimizeMeshes |
			aiProcess_GenUVCoords |
			aiProcess_FixInfacingNormals |
			aiProcess_RemoveRedundantMaterials
		);

		return scene;
	}

	static AssimpModelResult ParseModelFile(const std::filesystem::path& filepath)
	{
		PE_PROFILE_FUNCTION();
		AssimpModelResult result;
		
		Assimp::Importer importer;
		const aiScene* scene = GetAssimpScene(importer, filepath);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			PE_CORE_ERROR("Error loading model file at path: '{0}'", filepath.string());
			PE_CORE_ERROR("    - {0}", importer.GetErrorString());
			result.Success = false;
			return result;
		}

		// Read meshes
		unsigned int numMeshes = scene->mNumMeshes;
		result.Meshes.reserve(numMeshes);
		for (unsigned int i = 0; i < numMeshes; i++)
		{
			result.Meshes.push_back(ReadAssimpMesh(scene->mMeshes[i]));
		}

		result.Success = true;
		return result;
	}

	Ref<Mesh> MeshImporter::ImportMesh(AssetHandle handle, const AssetMetadata& metadata)
	{
		PE_PROFILE_FUNCTION();
		Ref<Mesh> mesh = LoadMesh(Project::GetAssetDirectory() / metadata.FilePath);
		mesh->Handle = handle;
		return mesh;
	}

	Ref<Model> MeshImporter::ImportModel(AssetHandle handle, const AssetMetadata& metadata)
	{
		PE_PROFILE_FUNCTION();
		Ref<Model> model = LoadModel(Project::GetAssetDirectory() / metadata.FilePath);

		if (model) {
			model->Handle = handle;
		}

		return model;
	}

	Ref<Mesh> MeshImporter::LoadMesh(const std::filesystem::path& filepath)
	{
		PE_PROFILE_FUNCTION();
		PE_CORE_ASSERT(filepath.extension() == ".pmesh", "Invalid file extension");

		std::ifstream stream = std::ifstream(filepath);
		std::stringstream ss;
		ss << stream.rdbuf();

		YAML::Node data = YAML::Load(ss.str());
		if (!data["Mesh"]) { return nullptr; }

		AssetHandle modelHandle = data["SourceModel"].as<uint64_t>();
		uint32_t meshIndex = data["Index"].as<uint32_t>();

		PE_CORE_ASSERT(AssetManager::IsAssetHandleValid(modelHandle), "Base model handle in mesh file is invalid");
		Ref<Model> baseModel = AssetManager::GetAsset<Model>(modelHandle);
		
		return baseModel->GetMesh(meshIndex);
	}

	Ref<Model> MeshImporter::LoadModel(const std::filesystem::path& filepath)
	{
		PE_PROFILE_FUNCTION();
		AssimpModelResult result = ParseModelFile(filepath);

		if (!result.Success)
		{
			return nullptr;
		}

		Ref<Model> resultModel = CreateRef<Model>();
		size_t numMeshes = result.Meshes.size();
		for (uint32_t i = 0; i < numMeshes; i++)
		{
			resultModel->m_Meshes.push_back(result.Meshes[i].LoadedMesh);
		}

		PE_CORE_INFO("Model load success from path: '{0}'", filepath.string());
		PE_CORE_INFO("    - Total meshes: {0}", numMeshes);
		return resultModel;
	}

	void MeshImporter::CreatePrefabFromImportedModel(AssetHandle modelHandle)
	{
		PE_PROFILE_FUNCTION();
		PE_CORE_ASSERT(AssetManager::IsAssetHandleValid(modelHandle), "Invalid asset handle");

		Ref<Model> model = AssetManager::GetAsset<Model>(modelHandle);
		const AssetMetadata& metadata = AssetManager::GetMetadata(modelHandle);

		std::filesystem::path assetDir = Project::GetAssetDirectory();

		// Import assimp scene again, required for reading node tree and material data
		Assimp::Importer importer;
		const aiScene* scene = GetAssimpScene(importer, assetDir / metadata.FilePath);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			PE_CORE_ERROR("Error loading model file at path: '{0}'", (assetDir / metadata.FilePath).string());
			PE_CORE_ERROR("    - {0}", importer.GetErrorString());
			return;
		}

		// Read node tree
		AssimpNodeTreeResult result;
		unsigned int rootNodeIndex = ReadAssimpNode(scene, scene->mRootNode, result);

		// Create .pmesh asset files
		size_t numMeshes = model->NumMeshes();
		std::vector<AssetHandle> meshHandles;
		for (uint32_t i = 0; i < numMeshes; i++)
		{
			meshHandles.push_back(CreateAndImportMeshFile(assetDir / metadata.FilePath, model->GetMesh(i), i, model->Handle, metadata.Persistent));
		}

		// Create .pmat asset files
		std::vector<AssetHandle> importedMaterials;
		MaterialImporter::ImportMaterialsFromModelFile(scene, Project::GetAssetDirectory() / metadata.FilePath, metadata.Persistent, importedMaterials);

		// Create prefab asset
		// -------------------

		// Create scene
		Ref<Scene> engineScene = CreateRef<Scene>();
		Entity rootEntity = CreateEntityFromNode(engineScene, result, rootNodeIndex, meshHandles, importedMaterials);
		Prefab newPrefab = Prefab::CreateFromEntity(rootEntity);

		std::filesystem::path savePath = metadata.FilePath.parent_path() / (metadata.FilePath.stem().string() + ".pfab");
		SceneImporter::SavePrefab(newPrefab, savePath);
		Project::GetActive()->GetEditorAssetManager()->ImportAssetFromFile(savePath, metadata.Persistent);
	}

	static ParsedMesh ParseAssimpMesh(const aiMesh* mesh)
	{
		PE_PROFILE_FUNCTION();

		// Read vertices
		unsigned int numVertices = mesh->mNumVertices;
		std::vector<MeshVertex> vertices = std::vector<MeshVertex>(numVertices);
		for (unsigned int i = 0; i < numVertices; i++)
		{
			MeshVertex& vertex = vertices[i];

			const aiVector3D& aiPosition = mesh->mVertices[i];
			const aiVector3D& aiNormal = mesh->mNormals[i];
			const aiVector3D& aiUV = mesh->mTextureCoords[0][i];
			const aiVector3D& aiTangent = mesh->mTangents[i];
			const aiVector3D& aiBitangent = mesh->mBitangents[i];

			vertex.Position = glm::vec3(aiPosition.x, aiPosition.y, aiPosition.z);
			vertex.Normal = glm::vec3(aiNormal.x, aiNormal.y, aiNormal.z);
			vertex.TexCoords = glm::vec2(aiUV.x, aiUV.y);
			vertex.Tangent = glm::vec3(aiTangent.x, aiTangent.y, aiTangent.z);
			vertex.Bitangent = glm::vec3(aiBitangent.x, aiBitangent.y, aiBitangent.z);
		}

		// Read indices
		std::vector<uint32_t> indices;
		unsigned int numFaces = mesh->mNumFaces;
		indices.reserve(numFaces * 3);
		for (unsigned int i = 0; i < numFaces; i++)
		{
			const aiFace& face = mesh->mFaces[i];
			unsigned int numIndices = face.mNumIndices;
			for (unsigned int j = 0; j < numIndices; j++)
			{
				indices.push_back(face.mIndices[j]);
			}
		}

		ParsedMesh result;
		result.Vertices = vertices;
		result.Indices = indices;

		return result;
	}

	Ref<ParsedModelLoadResult> MeshImporter::ParseModelFileRaw(const std::filesystem::path& filepath)
	{
		PE_PROFILE_FUNCTION();
		Ref<ParsedModelLoadResult> result = CreateRef<ParsedModelLoadResult>();

		Assimp::Importer importer;
		const aiScene* scene = GetAssimpScene(importer, filepath);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			PE_CORE_ERROR("Error loading model file at path: '{0}'", filepath.string());
			PE_CORE_ERROR("    - {0}", importer.GetErrorString());
			return nullptr;
		}

		// Read meshes
		unsigned int numMeshes = scene->mNumMeshes;
		result->ParsedMeshes.reserve(numMeshes);
		for (unsigned int i = 0; i < numMeshes; i++)
		{
			result->ParsedMeshes.push_back(ParseAssimpMesh(scene->mMeshes[i]));
		}

		return result;
	}

	std::filesystem::path MeshImporter::CreateMeshFile(const std::filesystem::path& baseModelFilepath, const Ref<Mesh>& loadedMesh, uint32_t meshIndex, const AssetHandle modelHandle, const bool persistent)
	{
		PE_PROFILE_FUNCTION();
		const MeshSpecification& spec = loadedMesh->GetSpec();

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Mesh" << YAML::Value << spec.Name;
		out << YAML::Key << "SourceModel" << YAML::Value << (uint64_t)modelHandle;
		out << YAML::Key << "Index" << YAML::Value << meshIndex;
		out << YAML::EndMap;

		std::filesystem::path savePath = baseModelFilepath.parent_path() / (spec.Name + ".pmesh");
		std::error_code error;
		std::filesystem::create_directories(savePath.parent_path(), error);
		std::ofstream fout = std::ofstream(savePath);
		fout << out.c_str();

		return savePath;
	}

	AssetHandle MeshImporter::CreateAndImportMeshFile(const std::filesystem::path& baseModelFilepath, const Ref<Mesh>& loadedMesh, uint32_t meshIndex, const AssetHandle modelHandle, const bool persistent)
	{
		std::filesystem::path savePath = CreateMeshFile(baseModelFilepath, loadedMesh, meshIndex, modelHandle, persistent).lexically_relative(Project::GetAssetDirectory());
		return Project::GetActive()->GetEditorAssetManager()->ImportAssetFromFile(savePath, persistent);
	}
}