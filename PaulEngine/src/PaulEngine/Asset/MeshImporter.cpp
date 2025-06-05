#include "pepch.h"
#include "MeshImporter.h"
#include "MaterialImporter.h"
#include "PaulEngine/Project/Project.h"
#include "PaulEngine/Asset/AssetManager.h"
#include "PaulEngine/Renderer/Renderer.h"

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

	struct AssimpNodeResult
	{
		std::string Name;
		glm::mat4 Transform;
		std::vector<unsigned int> MeshIndices; // indexed to aiScene mesh list
	};

	struct AssimpResult
	{
		bool Success = false;
		std::vector<AssimpMeshResult> Meshes = {};
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

	static void ReadAssimpNode(const aiNode* node, AssimpResult& result)
	{
		AssimpNodeResult nodeResult;
		nodeResult.Name = node->mName.C_Str();
		nodeResult.Transform = AssimpMat4ToGLMMat4(node->mTransformation);

		unsigned int numMeshes = node->mNumMeshes;
		nodeResult.MeshIndices.reserve(numMeshes);
		for (unsigned int i = 0; i < numMeshes; i++)
		{
			nodeResult.MeshIndices.push_back(node->mMeshes[i]);
		}
		result.Nodes.push_back(nodeResult);

		unsigned int numChildren = node->mNumChildren;
		for (unsigned int i = 0; i < numChildren; i++)
		{
			ReadAssimpNode(node->mChildren[i], result);
		}
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
		spec.UsageType = BufferUsage::STATIC_DRAW;

		switch (mesh->mPrimitiveTypes)
		{
		case aiPrimitiveType_POINT:
			spec.PrimitiveType = DrawPrimitive::POINTS;
			break;
		case aiPrimitiveType_LINE:
			spec.PrimitiveType = DrawPrimitive::LINES;
			break;
		case aiPrimitiveType_TRIANGLE:
			spec.PrimitiveType = DrawPrimitive::TRIANGLES;
			break;
		case aiPrimitiveType_POLYGON:
			spec.PrimitiveType = DrawPrimitive::None;
			PE_CORE_WARN("Unsupported primitive type 'Polygon' in model file");
			break;
			// not sure on this one
			//case aiPrimitiveType_NGONEncodingFlag:
			//	spec.PrimitiveType = DrawPrimitive::TRIANGLE_FAN;
			//	break;
		}

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

	static AssimpResult ParseModelFile(const std::filesystem::path& filepath)
	{
		PE_PROFILE_FUNCTION();
		AssimpResult result;
		
		Assimp::Importer importer;
		bool success = importer.ValidateFlags(
			aiProcess_GenNormals |
			aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_OptimizeMeshes |
			aiProcess_OptimizeGraph |
			aiProcess_GenUVCoords
		);
		const aiScene* scene = importer.ReadFile(filepath.string(),
			aiProcess_GenNormals |
			aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_OptimizeMeshes |
			aiProcess_OptimizeGraph |
			aiProcess_GenUVCoords
		);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			PE_CORE_ERROR("Error loading model file at path: '{0}'", filepath.string());
			PE_CORE_ERROR("    - {0}", importer.GetErrorString());
			result.Success = false;
			return result;
		}

		// Read node tree
		ReadAssimpNode(scene->mRootNode, result);

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

			// If the asset handle is not already valid (registered), then this is the first time the asset
			// is being registered, therefore mesh and material files need to be created.
			// If this asset has been registered, then the associated mesh and material files have already been imported
			if (!AssetManager::IsAssetHandleValid(handle))
			{
				// Create .pmesh asset files
				size_t numMeshes = model->NumMeshes();
				for (uint32_t i = 0; i < numMeshes; i++)
				{
					CreateMeshFile(Project::GetAssetDirectory() / metadata.FilePath, model->GetMesh(i), i, model->Handle, metadata.Persistent);
				}

				// Create .pmat asset files
				MaterialImporter::ImportMaterialsFromModelFile(Project::GetAssetDirectory() / metadata.FilePath, metadata.Persistent);
			}
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
		AssimpResult result = ParseModelFile(filepath);

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

	void MeshImporter::CreateMeshFile(const std::filesystem::path& baseModelFilepath, const Ref<Mesh>& loadedMesh, uint32_t meshIndex, const AssetHandle modelHandle, const bool persistent)
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
	}
}