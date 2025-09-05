#include "pepch.h"
#include "PaulEngine/Asset/AssetManager.h"
#include "MaterialImporter.h"
#include "PaulEngine/Renderer/Resource/Buffer.h"
#include "PaulEngine/Project/Project.h"
#include "PaulEngine/Renderer/Renderer.h"
#include "PaulEngine/Renderer/Asset/Texture.h"

#include <yaml-cpp/yaml.h>
#include "PaulEngine/Utils/YamlConversions.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace PaulEngine
{
	namespace MaterialImporterUtils
	{
		static void WriteUniformBufferStorageObject(YAML::Emitter& out, UBOShaderParameterTypeStorage* uboStorageParameter)
		{
			Ref<UniformBufferStorage> ubo = uboStorageParameter->UBO();

			out << YAML::BeginMap;
			out << YAML::Key << "Binding" << YAML::Value << ubo->GetBinding();
			
			out << YAML::Key << "Layout" << YAML::Value;

			out << YAML::BeginSeq;
			const std::vector<BufferElement>& layout = ubo->GetMembers();
			for (const BufferElement& e : layout) {
				out << YAML::BeginMap;

				ShaderDataType type = e.Type;
				const std::string& name = e.Name;
				out << YAML::Key << "Type" << YAML::Value << ShaderDataTypeToString(type);
				out << YAML::Key << "Name" << YAML::Value << name;
				out << YAML::Key << "Value" << YAML::Value;

				switch (type)
				{
					case ShaderDataType::None:
						out << "NULL";
						break;
					case ShaderDataType::Float:
					{
						float data = 0.0f;
						ubo->ReadLocalDataAs(name, data);
						out << data;
						break;
					}
					case ShaderDataType::Float2:
					{
						glm::vec2 data = glm::vec2(0.0f);
						ubo->ReadLocalDataAs(name, data);
						out << data;
						break;
					}
					case ShaderDataType::Float3:
					{
						glm::vec3 data = glm::vec3(0.0f);
						ubo->ReadLocalDataAs(name, data);
						out << data;
						break;
					}
					case ShaderDataType::Float4:
					{
						glm::vec4 data = glm::vec4(0.0f);
						ubo->ReadLocalDataAs(name, data);
						out << data;
						break;
					}
					case ShaderDataType::Mat3:
					{
						glm::mat3 data = glm::mat3(0.0f);
						ubo->ReadLocalDataAs(name, data);
						out << data;
						break;
					}
					case ShaderDataType::Mat4:
					{
						glm::mat4 data = glm::mat4(0.0f);
						ubo->ReadLocalDataAs(name, data);
						out << data;
						break;
					}
					case ShaderDataType::Int:
					{
						int data = 0;
						ubo->ReadLocalDataAs(name, data);
						out << data;
						break;
					}
					case ShaderDataType::Int2:
					{
						glm::ivec2 data = glm::ivec2(0);
						ubo->ReadLocalDataAs(name, data);
						out << data;
						break;
					}
					case ShaderDataType::Int3:
					{
						glm::ivec3 data = glm::ivec3(0);
						ubo->ReadLocalDataAs(name, data);
						out << data;
						break;
					}
					case ShaderDataType::Int4:
					{
						glm::ivec4 data = glm::ivec4(0);
						ubo->ReadLocalDataAs(name, data);
						out << data;
						break;
					}
					case ShaderDataType::Bool:
					{
						bool data = 0;
						ubo->ReadLocalDataAs(name, data);
						out << data;
						break;
					}
				}

				out << YAML::EndMap;
			}
			out << YAML::EndSeq;

			out << YAML::EndMap;
		}
		static Ref<UBOShaderParameterTypeStorage> ReadUniformBufferStorageObject(YAML::Node& valueNode)
		{
			uint32_t binding = valueNode["Binding"].as<uint64_t>();

			std::vector<BufferElement> layout;

			YAML::Node layoutNode = valueNode["Layout"];
			for (YAML::Node layoutEntry : layoutNode) {
				std::string typeString = layoutEntry["Type"].as<std::string>();
				ShaderDataType type = StringToShaderDataType(typeString);
				std::string name = layoutEntry["Name"].as<std::string>();
				layout.emplace_back(type, name);
			}
			Ref<UBOShaderParameterTypeStorage> ubo = CreateRef<UBOShaderParameterTypeStorage>(layout, binding);

			for (YAML::Node layoutEntry : layoutNode) {
				std::string name = layoutEntry["Name"].as<std::string>();
				std::string typeString = layoutEntry["Type"].as<std::string>();
				ShaderDataType type = StringToShaderDataType(typeString);
				switch (type)
				{
					case ShaderDataType::None:
						continue;
						break;
					case ShaderDataType::Float:
					{
						float data = layoutEntry["Value"].as<float>();
						ubo->UBO()->SetLocalData(name, data);
						break;
					}
					case ShaderDataType::Float2:
					{
						glm::vec2 data = layoutEntry["Value"].as<glm::vec2>();
						ubo->UBO()->SetLocalData(name, data);
						break;
					}
					case ShaderDataType::Float3:
					{
						glm::vec3 data = layoutEntry["Value"].as<glm::vec3>();
						ubo->UBO()->SetLocalData(name, data);
						break;
					}
					case ShaderDataType::Float4:
					{
						glm::vec4 data = layoutEntry["Value"].as<glm::vec4>();
						ubo->UBO()->SetLocalData(name, data);
						break;
					}
					case ShaderDataType::Mat3:
					{
						glm::mat3 data = layoutEntry["Value"].as<glm::mat3>();
						ubo->UBO()->SetLocalData(name, data);
						break;
					}
					case ShaderDataType::Mat4:
					{
						glm::mat4 data = layoutEntry["Value"].as<glm::mat4>();
						ubo->UBO()->SetLocalData(name, data);
						break;
					}
					case ShaderDataType::Int:
					{
						int data = layoutEntry["Value"].as<int>();
						ubo->UBO()->SetLocalData(name, data);
						break;
					}
					case ShaderDataType::Int2:
					{
						glm::ivec2 data = layoutEntry["Value"].as<glm::ivec2>();
						ubo->UBO()->SetLocalData(name, data);
						break;
					}
					case ShaderDataType::Int3:
					{
						glm::ivec3 data = layoutEntry["Value"].as<glm::ivec3>();
						ubo->UBO()->SetLocalData(name, data);
						break;
					}
					case ShaderDataType::Int4:
					{
						glm::ivec4 data = layoutEntry["Value"].as<glm::ivec4>();
						ubo->UBO()->SetLocalData(name, data);
						break;
					}
					case ShaderDataType::Bool:
					{
						bool data = layoutEntry["Value"].as<bool>();
						ubo->UBO()->SetLocalData(name, data);
						break;
					}
				}
			}
			return ubo;
		}

		static void WriteSampler2DObject(YAML::Emitter& out, Sampler2DShaderParameterTypeStorage* sampler2DStorageParameter)
		{
			out << YAML::BeginMap;

			out << YAML::Key << "Binding" << YAML::Value << sampler2DStorageParameter->GetBinding();
			out << YAML::Key << "TextureHandle" << YAML::Value << sampler2DStorageParameter->TextureHandle;

			out << YAML::EndMap;
		}
		static Ref<Sampler2DShaderParameterTypeStorage> ReadSampler2DObject(YAML::Node& valueNode)
		{
			uint32_t binding = valueNode["Binding"].as<uint64_t>();
			AssetHandle textureHandle = valueNode["TextureHandle"].as<AssetHandle>();

			return CreateRef<Sampler2DShaderParameterTypeStorage>(textureHandle, binding);
		}

		static void WriteSampler2DArrayObject(YAML::Emitter& out, Sampler2DArrayShaderParameterTypeStorage* sampler2DArrayStorageParameter)
		{
			out << YAML::BeginMap;

			out << YAML::Key << "Binding" << YAML::Value << sampler2DArrayStorageParameter->GetBinding();
			out << YAML::Key << "TextureArrayHandle" << YAML::Value << sampler2DArrayStorageParameter->TextureArrayHandle;

			out << YAML::EndMap;
		}
		static Ref<Sampler2DArrayShaderParameterTypeStorage> ReadSampler2DArrayObject(YAML::Node& valueNode)
		{
			uint32_t binding = valueNode["Binding"].as<uint64_t>();
			AssetHandle textureArrayHandle = valueNode["TextureArrayHandle"].as<AssetHandle>();

			return CreateRef<Sampler2DArrayShaderParameterTypeStorage>(textureArrayHandle, binding);
		}

		static void WriteSamplerCubeObject(YAML::Emitter& out, SamplerCubeShaderParameterTypeStorage* samplerCubeStorageParameter)
		{
			out << YAML::BeginMap;

			out << YAML::Key << "Binding" << YAML::Value << samplerCubeStorageParameter->GetBinding();
			out << YAML::Key << "TextureHandle" << YAML::Value << samplerCubeStorageParameter->TextureHandle;

			out << YAML::EndMap;
		}
		static Ref<SamplerCubeShaderParameterTypeStorage> ReadSamplerCubeObject(YAML::Node& valueNode)
		{
			uint32_t binding = valueNode["Binding"].as<uint64_t>();
			AssetHandle textureHandle = valueNode["TextureHandle"].as<AssetHandle>();

			return CreateRef<SamplerCubeShaderParameterTypeStorage>(textureHandle, binding);
		}
	}

	struct AssimpMaterialResult
	{
		std::string Name;
		Ref<Material> LoadedMaterial;
	};

	struct AssimpTextureResult
	{
		bool TextureFound = false;
		std::filesystem::path RelativePath;
		TextureSpecification Spec;
		glm::vec2 TextureScaling = glm::vec2(1.0f);
	};

	// TODO: Update texture importer so that a lot of these retrieved values can actually be used
	static AssimpTextureResult ReadAssimpTexture(const aiMaterial* material, const aiTextureType type)
	{
		PE_PROFILE_FUNCTION();

		AssimpTextureResult result;
		result.TextureFound = false;

		unsigned int textureCount = material->GetTextureCount(type);
		if (textureCount > 0)
		{
			aiString filestring;
			aiTextureMapping mapping;
			unsigned int uvIndex;
			ai_real blend;
			aiTextureOp op;
			aiTextureMapMode mapmode[3];
			if (material->GetTexture(type, 0, &filestring, &mapping, &uvIndex, &blend, &op, &mapmode[0]) == aiReturn_SUCCESS)
			{
				result.RelativePath = filestring.C_Str();
				result.TextureFound = true;

				aiUVTransform transform;
				if (material->Get(AI_MATKEY_UVTRANSFORM(type, 0), transform) == aiReturn_SUCCESS)
				{
					result.TextureScaling = glm::vec2(transform.mScaling.x, transform.mScaling.y);
				}

				TextureSpecification spec;
				ImageWrap wrap[3] = { ImageWrap::REPEAT, ImageWrap::REPEAT, ImageWrap::REPEAT };
				for (int i = 0; i < 3; i++)
				{
					switch (mapmode[i])
					{
					case aiTextureMapMode_Wrap:
						// A texture coordinate u|v is translated to u%1|v%1
						wrap[i] = ImageWrap::REPEAT;
						break;
					case aiTextureMapMode_Clamp:
						// Texture coordinates outside [0...1] are clamped to the nearest valid value.
						wrap[i] = ImageWrap::CLAMP_TO_EDGE;
						break;
					case aiTextureMapMode_Decal:
						// If the texture coordinates for a pixel are outside[0...1] the texture is not applied to that pixel
						wrap[i] = ImageWrap::CLAMP_TO_BORDER;
						break;
					case aiTextureMapMode_Mirror:
						// A texture coordinate u|v becomes u%1|v%1 if (u-(u%1))%2 is zero and 1 - (u % 1) | 1 - (v % 1) otherwise
						wrap[i] = ImageWrap::MIRRORED_REPEAT;
						break;
					}
				}
				spec.Wrap_S = wrap[0];
				spec.Wrap_T = wrap[1];
				spec.Wrap_R = wrap[2];

				result.Spec = spec;
			}
		}

		return result;
	}

	static Ref<Material> BuildAssimpMaterial(const aiMaterial* material, const std::filesystem::path& sourcePath, bool persistent)
	{
		PE_PROFILE_FUNCTION();
		const AssetHandle shaderHandle = Renderer::GetDefaultLitShader();
		Ref<Material> engineMaterial = CreateRef<Material>(shaderHandle);

		// Read material properties
		// ------------------------
		aiColor4D baseColour = aiColor4D(1.0, 1.0, 1.0, 1.0);
		float opacity = 1.0f;
		if (aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &baseColour) != aiReturn_SUCCESS)
		{
			baseColour = aiColor4D(1.0, 1.0, 1.0, 1.0);
		}
		if (material->Get(AI_MATKEY_OPACITY, opacity) != aiReturn_SUCCESS)
		{
			opacity = 1.0f;
		}
		glm::vec4 Albedo = glm::vec4(baseColour.r, baseColour.g, baseColour.b, opacity);

		aiColor4D specularColour = baseColour;
		if (material->Get(AI_MATKEY_COLOR_SPECULAR, specularColour) != aiReturn_SUCCESS)
		{
			specularColour = baseColour;
		}
		glm::vec4 Specular = glm::vec4(specularColour.r, specularColour.g, specularColour.b, 1.0f);

		float Shininess = 64.0f;
		if (material->Get(AI_MATKEY_SHININESS, Shininess) != aiReturn_SUCCESS)
		{
			Shininess = 64.0f;
		}
		
		// Not sure if this is correct but its the only value I could find in assimp that might relate to height scaling
		float HeightScale = 0.1f;
		if (material->Get(AI_MATKEY_BUMPSCALING, HeightScale) != aiReturn_SUCCESS)
		{
			HeightScale = 0.1f;
		}

		AssimpTextureResult albedoResult = ReadAssimpTexture(material, aiTextureType_DIFFUSE);
		AssimpTextureResult specularResult = ReadAssimpTexture(material, aiTextureType_SPECULAR);
		AssimpTextureResult normalResult = ReadAssimpTexture(material, aiTextureType_NORMALS);
		AssimpTextureResult displacementResult = ReadAssimpTexture(material, aiTextureType_DISPLACEMENT);

		// Import textures from files
		AssetHandle AlbedoMap = 0;
		AssetHandle SpecularMap = 0;
		AssetHandle NormalMap = 0;
		AssetHandle DisplacementMap = 0;

		EditorAssetManager* assetManager = Project::GetActive()->GetEditorAssetManager().get();
		const std::filesystem::path assetsPath = Project::GetAssetDirectory();
		const std::filesystem::path dirPath = sourcePath.parent_path().lexically_relative(assetsPath);

		if (albedoResult.TextureFound) { AlbedoMap = assetManager->ImportAssetFromFile(dirPath / albedoResult.RelativePath, persistent); }
		if (specularResult.TextureFound) { SpecularMap = assetManager->ImportAssetFromFile(dirPath / specularResult.RelativePath, persistent); }
		if (normalResult.TextureFound) { NormalMap = assetManager->ImportAssetFromFile(dirPath / normalResult.RelativePath, persistent); }
		if (displacementResult.TextureFound) { DisplacementMap = assetManager->ImportAssetFromFile(dirPath / displacementResult.RelativePath, persistent); }

		int UseNormalMap = (AssetManager::IsAssetHandleValid(NormalMap)) ? 1 : 0;
		int UseDisplacementMap = (AssetManager::IsAssetHandleValid(DisplacementMap)) ? 1 : 0;

		// Default material doesn't offer per-texture scaling, so the diffuse texture scaling will be used across all material textures
		glm::vec2 TextureScale = albedoResult.TextureScaling;

		// Apply material parameters
		// -------------------------
		UBOShaderParameterTypeStorage* matValuesUBO = engineMaterial->GetParameter<UBOShaderParameterTypeStorage>("MaterialValues");
		Ref<UniformBufferStorage> uboStorage = matValuesUBO->UBO();
		uboStorage->SetLocalData("Albedo", Albedo);
		uboStorage->SetLocalData("Specular", Specular);
		uboStorage->SetLocalData("TextureScale", TextureScale);
		uboStorage->SetLocalData("Shininess", Shininess);
		uboStorage->SetLocalData("HeightScale", HeightScale);
		uboStorage->SetLocalData("UseNormalMap", UseNormalMap);
		uboStorage->SetLocalData("UseDisplacementMap", UseDisplacementMap);

		engineMaterial->GetParameter<Sampler2DShaderParameterTypeStorage>("AlbedoMap")->TextureHandle = AlbedoMap;
		engineMaterial->GetParameter<Sampler2DShaderParameterTypeStorage>("SpecularMap")->TextureHandle = SpecularMap;
		engineMaterial->GetParameter<Sampler2DShaderParameterTypeStorage>("NormalMap")->TextureHandle = NormalMap;
		engineMaterial->GetParameter<Sampler2DShaderParameterTypeStorage>("DisplacementMap")->TextureHandle = DisplacementMap;
		
		return engineMaterial;
	}

	static Ref<Material> BuildAssimpMaterialAsPBR(const aiMaterial* material, const std::filesystem::path& sourcePath, bool persistent)
	{
		PE_PROFILE_FUNCTION();
		const AssetHandle shaderHandle = Renderer::GetDefaultLitPBRShader();
		Ref<Material> engineMaterial = CreateRef<Material>(shaderHandle);

		// Read material properties
		// ------------------------
		aiColor3D baseColour = aiColor3D(1.0f, 1.0f, 1.0f);
		float opacity = 1.0f;
		if (material->Get(AI_MATKEY_COLOR_DIFFUSE, baseColour) != aiReturn_SUCCESS)
		{
			baseColour = aiColor3D(1.0f, 1.0f, 1.0f);
		}
		if (material->Get(AI_MATKEY_OPACITY, opacity) != aiReturn_SUCCESS)
		{
			opacity = 1.0f;
		}
		glm::vec4 Albedo = glm::vec4(baseColour.r, baseColour.g, baseColour.b, opacity);

		float Metalness = 0.0f;
		if (material->Get(AI_MATKEY_METALLIC_FACTOR, Metalness) != aiReturn_SUCCESS)
		{
			Metalness = 0.0f;
		}

		float Roughness = 0.5f;
		if (material->Get(AI_MATKEY_ROUGHNESS_FACTOR, Roughness) != aiReturn_SUCCESS)
		{
			Roughness = 0.5f;
		}

		aiColor3D ambientColour = aiColor3D(1.0f, 1.0f, 1.0f);
		if (material->Get(AI_MATKEY_COLOR_AMBIENT, ambientColour) != aiReturn_SUCCESS)
		{
			ambientColour = aiColor3D(1.0f, 1.0f, 1.0f);
		}
		float AO = (0.299f * ambientColour.r) + (0.587f * ambientColour.g) + (0.114 * ambientColour.b); // grayscale value

		// Not sure if this is correct but its the only value I could find in assimp that might relate to height scaling
		float HeightScale = 0.1f;
		if (material->Get(AI_MATKEY_BUMPSCALING, HeightScale) != aiReturn_SUCCESS)
		{
			HeightScale = 0.1f;
		}

		AssimpTextureResult albedoResult = ReadAssimpTexture(material, aiTextureType_DIFFUSE);
		AssimpTextureResult normalResult = ReadAssimpTexture(material, aiTextureType_NORMALS);
		AssimpTextureResult metallicResult = ReadAssimpTexture(material, aiTextureType_METALNESS);
		AssimpTextureResult roughnessResult = ReadAssimpTexture(material, aiTextureType_DIFFUSE_ROUGHNESS);
		AssimpTextureResult aoResult = ReadAssimpTexture(material, aiTextureType_AMBIENT);
		AssimpTextureResult displacementResult = ReadAssimpTexture(material, aiTextureType_DISPLACEMENT);

		// Import textures from files
		AssetHandle AlbedoMap = 0;
		AssetHandle NormalMap = 0;
		AssetHandle MetallicMap = 0;
		AssetHandle RoughnessMap = 0;
		AssetHandle AOMap = 0;
		AssetHandle DisplacementMap = 0;

		EditorAssetManager* assetManager = Project::GetActive()->GetEditorAssetManager().get();
		const std::filesystem::path assetsPath = Project::GetAssetDirectory();
		const std::filesystem::path dirPath = sourcePath.parent_path().lexically_relative(assetsPath);

		if (albedoResult.TextureFound) { AlbedoMap = assetManager->ImportAssetFromFile(dirPath / albedoResult.RelativePath, persistent); }
		if (normalResult.TextureFound) { NormalMap = assetManager->ImportAssetFromFile(dirPath / normalResult.RelativePath, persistent); }
		if (metallicResult.TextureFound) { MetallicMap = assetManager->ImportAssetFromFile(dirPath / metallicResult.RelativePath, persistent); }
		if (roughnessResult.TextureFound) { RoughnessMap = assetManager->ImportAssetFromFile(dirPath / roughnessResult.RelativePath, persistent); }
		if (aoResult.TextureFound) { AOMap = assetManager->ImportAssetFromFile(dirPath / aoResult.RelativePath, persistent); }
		if (displacementResult.TextureFound) { DisplacementMap = assetManager->ImportAssetFromFile(dirPath / displacementResult.RelativePath, persistent); }

		int UseNormalMap = (AssetManager::IsAssetHandleValid(NormalMap)) ? 1 : 0;
		int UseDisplacementMap = (AssetManager::IsAssetHandleValid(DisplacementMap)) ? 1 : 0;

		// Default material doesn't offer per-texture scaling, so the diffuse texture scaling will be used across all material textures
		glm::vec2 TextureScale = albedoResult.TextureScaling;

		// Apply material parameters
		// -------------------------
		UBOShaderParameterTypeStorage* matValuesUBO = engineMaterial->GetParameter<UBOShaderParameterTypeStorage>("MaterialValues");
		Ref<UniformBufferStorage> uboStorage = matValuesUBO->UBO();
		uboStorage->SetLocalData("Albedo", Albedo);
		uboStorage->SetLocalData("Metalness", Metalness);
		uboStorage->SetLocalData("Roughness", Roughness);
		uboStorage->SetLocalData("AO", AO);
		uboStorage->SetLocalData("HeightScale", HeightScale);
		uboStorage->SetLocalData("TextureScale", TextureScale);
		uboStorage->SetLocalData("UseNormalMap", UseNormalMap);
		uboStorage->SetLocalData("UseDisplacementMap", UseDisplacementMap);

		engineMaterial->GetParameter<Sampler2DShaderParameterTypeStorage>("AlbedoMap")->TextureHandle = AlbedoMap;
		engineMaterial->GetParameter<Sampler2DShaderParameterTypeStorage>("NormalMap")->TextureHandle = NormalMap;
		engineMaterial->GetParameter<Sampler2DShaderParameterTypeStorage>("MetallicMap")->TextureHandle = MetallicMap;
		engineMaterial->GetParameter<Sampler2DShaderParameterTypeStorage>("RoughnessMap")->TextureHandle = RoughnessMap;
		engineMaterial->GetParameter<Sampler2DShaderParameterTypeStorage>("AOMap")->TextureHandle = AOMap;
		engineMaterial->GetParameter<Sampler2DShaderParameterTypeStorage>("DisplacementMap")->TextureHandle = DisplacementMap;

		return engineMaterial;
	}

	static AssimpMaterialResult ReadAssimpMaterial(const aiMaterial* material, const std::filesystem::path& sourcePath, bool persistent)
	{
		PE_PROFILE_FUNCTION();
		AssimpMaterialResult result;
		result.Name = material->GetName().C_Str();

		// assimp->code/AssetLib/Obj/ObjFileImporter/line 598 defines the translations from MTL illum values to assimp lighting models
		// The assimp MTL importer only supports MTL illum values from 0 - 2 and does not provide any PBR lighting models, only "noshading, gouraud, and phong"
		// Therefore, because of the limitations of MTL files, aiShadingMode_Gouraud will be used to define a PBR lighting model

		// Get shading model
		aiShadingMode assimpShading;
		if (aiGetMaterialInteger(material, AI_MATKEY_SHADING_MODEL, (int*)&assimpShading) != aiReturn_SUCCESS)
		{
			assimpShading = aiShadingMode_Blinn;
		}

		const bool shouldUsePBRShader = (assimpShading == aiShadingMode_PBR_BRDF || assimpShading == aiShadingMode_CookTorrance || assimpShading == aiShadingMode_Gouraud);
		result.LoadedMaterial = (shouldUsePBRShader) ? BuildAssimpMaterialAsPBR(material, sourcePath, persistent) : BuildAssimpMaterial(material, sourcePath, persistent);

		return result;
	}

	static std::string ShaderParameterTypeToString(ShaderParameterType type) {
		switch (type)
		{
			case ShaderParameterType::None: return "None";
			case ShaderParameterType::UBO: return "UBO";
			case ShaderParameterType::Sampler2D: return "Sampler2D";
			case ShaderParameterType::Sampler2DArray: return "Sampler2DArray";
			case ShaderParameterType::SamplerCube: return "SamplerCube";
		}
		PE_CORE_ASSERT(false, "Unknown shader parameter type!");
		return "";
	}

	static ShaderParameterType StringToShaderParameterType(const std::string& input) {
		if (input == "None") { return ShaderParameterType::None; }
		else if (input == "UBO") { return ShaderParameterType::UBO; }
		else if (input == "Sampler2D") { return ShaderParameterType::Sampler2D; }
		else if (input == "Sampler2DArray") { return ShaderParameterType::Sampler2DArray; }
		else if (input == "SamplerCube") { return ShaderParameterType::SamplerCube; }
		PE_CORE_ASSERT(false, "Unknown shader parameter type!");
		return ShaderParameterType::None;
	}

	Ref<Material> MaterialImporter::ImportMaterial(AssetHandle handle, const AssetMetadata& metadata)
	{
		PE_PROFILE_FUNCTION();
		Ref<Material> material = LoadMaterial(Project::GetAssetDirectory() / metadata.FilePath);
		material->Handle = handle;
		return material;
	}

	Ref<Material> MaterialImporter::LoadMaterial(const std::filesystem::path& filepath)
	{
		PE_PROFILE_FUNCTION();
		PE_CORE_ASSERT(filepath.extension() == ".pmat", "Invalid file extension");

		std::ifstream stream = std::ifstream(filepath);
		std::stringstream ss;
		ss << stream.rdbuf();

		YAML::Node data = YAML::Load(ss.str());
		if (!data["MaterialAsset"]) { return nullptr; }

		std::string materialName = data["MaterialAsset"].as<std::string>();
		PE_CORE_TRACE("Deserializing material '{0}'", materialName);

		AssetHandle shaderHandle = data["ShaderHandle"].as<AssetHandle>();
		Material material = Material(shaderHandle);

		YAML::Node shaderParams = data["ShaderParams"];
		if (shaderParams) {
			for (YAML::Node shaderParameter : shaderParams) {
				std::string paramName = shaderParameter["Name"].as<std::string>();
				ShaderParameterType type = StringToShaderParameterType(shaderParameter["Type"].as<std::string>());

				YAML::Node value = shaderParameter["Value"];
				switch (type)
				{
				case ShaderParameterType::None:
					break;
				case ShaderParameterType::UBO:
					material.SetParameter(paramName, MaterialImporterUtils::ReadUniformBufferStorageObject(value));
					break;
				case ShaderParameterType::Sampler2D:
					material.SetParameter(paramName, MaterialImporterUtils::ReadSampler2DObject(value));
					break;
				case ShaderParameterType::Sampler2DArray:
					material.SetParameter(paramName, MaterialImporterUtils::ReadSampler2DArrayObject(value));
					break;
				case ShaderParameterType::SamplerCube:
					material.SetParameter(paramName, MaterialImporterUtils::ReadSamplerCubeObject(value));
					break;
				}

			}
		}

		return CreateRef<Material>(material);
	}

	void MaterialImporter::SaveMaterial(const Ref<Material> material, const std::filesystem::path& filepath)
	{
		PE_PROFILE_FUNCTION();
		PE_CORE_ASSERT(filepath.extension() == ".pmat", "Invalid file extension");

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "MaterialAsset" << YAML::Value << filepath.stem().string();
		out << YAML::Key << "ShaderHandle" << YAML::Value << material->m_ShaderHandle;
		out << YAML::Key << "ShaderParams" << YAML::Value << YAML::BeginSeq;

		for (auto& [name, parameter] : material->m_BindingParameters) {
			out << YAML::BeginMap;

			out << YAML::Key << "Name" << YAML::Value << name;

			ShaderParameterType type = parameter->GetType();
			out << YAML::Key << "Type" << YAML::Value << ShaderParameterTypeToString(type);

			out << YAML::Key << "Value";

			switch (type)
			{
				case ShaderParameterType::None:
				{
					out << YAML::Value << "Error Type";
					break;
				}
				case ShaderParameterType::UBO:
				{
					UBOShaderParameterTypeStorage* uboStorage = dynamic_cast<UBOShaderParameterTypeStorage*>(parameter.get());
					MaterialImporterUtils::WriteUniformBufferStorageObject(out, uboStorage);
					break;
				}
				case ShaderParameterType::Sampler2D:
				{
					MaterialImporterUtils::WriteSampler2DObject(out, dynamic_cast<Sampler2DShaderParameterTypeStorage*>(parameter.get()));
					break;
				}
				case ShaderParameterType::Sampler2DArray:
				{
					MaterialImporterUtils::WriteSampler2DArrayObject(out, dynamic_cast<Sampler2DArrayShaderParameterTypeStorage*>(parameter.get()));
					break;
				}
				case ShaderParameterType::SamplerCube:
				{
					MaterialImporterUtils::WriteSamplerCubeObject(out, dynamic_cast<SamplerCubeShaderParameterTypeStorage*>(parameter.get()));
					break;
				}
			}

			out << YAML::EndMap;
		}
		out << YAML::EndSeq;

		out << YAML::EndMap;

		std::error_code error;
		std::filesystem::create_directories(filepath.parent_path(), error);
		std::ofstream fout = std::ofstream(filepath);
		fout << out.c_str();
	}

	void MaterialImporter::ImportMaterialsFromModelFile(const std::filesystem::path& filepath, bool persistent)
	{
		std::vector<AssetHandle> out;
		ImportMaterialsFromModelFile(filepath, persistent, out);
	}

	void MaterialImporter::ImportMaterialsFromModelFile(const std::filesystem::path& filepath, bool persistent, std::vector<AssetHandle>& out_results)
	{
		PE_PROFILE_FUNCTION();
		out_results.clear();
		Assimp::Importer importer;
		bool success = importer.ValidateFlags(aiProcess_RemoveRedundantMaterials);

		const aiScene* scene = importer.ReadFile(filepath.string(), aiProcess_RemoveRedundantMaterials);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			PE_CORE_ERROR("Error loading model file at path: '{0}'", filepath.string());
			PE_CORE_ERROR("    - {0}", importer.GetErrorString());
			return;
		}

		ImportMaterialsFromModelFile(scene, filepath, persistent, out_results);
	}

	void MaterialImporter::ImportMaterialsFromModelFile(const aiScene* scene, const std::filesystem::path& filepath, bool persistent, std::vector<AssetHandle>& out_results)
	{
		// Read and import materials
		EditorAssetManager* editorAssetManager = Project::GetActive()->GetEditorAssetManager().get();
		unsigned int numMaterials = scene->mNumMaterials;
		const std::filesystem::path dirPath = filepath.parent_path();
		const std::filesystem::path relativeModelDir = dirPath.lexically_relative(Project::GetAssetDirectory());
		for (unsigned int i = 0; i < numMaterials; i++)
		{
			aiMaterial* material = scene->mMaterials[i];
			AssimpMaterialResult result = ReadAssimpMaterial(material, filepath, persistent);
			std::filesystem::path matPath = dirPath / (result.Name + ".pmat");
			SaveMaterial(result.LoadedMaterial, matPath);
			AssetHandle imported = editorAssetManager->ImportAssetFromFile(matPath.lexically_relative(Project::GetAssetDirectory()), persistent);
			out_results.push_back(imported);
		}
	}
}