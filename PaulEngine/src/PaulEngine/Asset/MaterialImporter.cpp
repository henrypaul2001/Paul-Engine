#include "pepch.h"
#include "MaterialImporter.h"
#include "PaulEngine/Renderer/Buffer.h"

#include "PaulEngine/Project/Project.h"

#include <yaml-cpp/yaml.h>

#include "PaulEngine/Utils/YamlConversions.h"

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
			const std::vector<UniformBufferStorage::BufferElement>& layout = ubo->GetMembers();
			for (const UniformBufferStorage::BufferElement& e : layout) {
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
						ubo->ReadLocalDataAs(name, &data);
						out << data;
						break;
					}
					case ShaderDataType::Float2:
					{
						glm::vec2 data = glm::vec2(0.0f);
						ubo->ReadLocalDataAs(name, &data);
						out << data;
						break;
					}
					case ShaderDataType::Float3:
					{
						glm::vec3 data = glm::vec3(0.0f);
						ubo->ReadLocalDataAs(name, &data);
						out << data;
						break;
					}
					case ShaderDataType::Float4:
					{
						glm::vec4 data = glm::vec4(0.0f);
						ubo->ReadLocalDataAs(name, &data);
						out << data;
						break;
					}
					case ShaderDataType::Mat3:
					{
						glm::mat3 data = glm::mat3(0.0f);
						ubo->ReadLocalDataAs(name, &data);
						out << data;
						break;
					}
					case ShaderDataType::Mat4:
					{
						glm::mat4 data = glm::mat4(0.0f);
						ubo->ReadLocalDataAs(name, &data);
						out << data;
						break;
					}
					case ShaderDataType::Int:
					{
						int data = 0;
						ubo->ReadLocalDataAs(name, &data);
						out << data;
						break;
					}
					case ShaderDataType::Int2:
					{
						glm::ivec2 data = glm::ivec2(0);
						ubo->ReadLocalDataAs(name, &data);
						out << data;
						break;
					}
					case ShaderDataType::Int3:
					{
						glm::ivec3 data = glm::ivec3(0);
						ubo->ReadLocalDataAs(name, &data);
						out << data;
						break;
					}
					case ShaderDataType::Int4:
					{
						glm::ivec4 data = glm::ivec4(0);
						ubo->ReadLocalDataAs(name, &data);
						out << data;
						break;
					}
					case ShaderDataType::Bool:
					{
						bool data = 0;
						ubo->ReadLocalDataAs(name, &data);
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

			std::vector<UniformBufferStorage::BufferElement> layout;

			YAML::Node layoutNode = valueNode["Layout"];
			for (YAML::Node layoutEntry : layoutNode) {
				ShaderDataType type = StringToShaderDataType(layoutEntry["Type"].as<std::string>());
				std::string name = layoutEntry["Name"].as<std::string>();
				layout.emplace_back(name, type);
			}
			Ref<UBOShaderParameterTypeStorage> ubo = CreateRef<UBOShaderParameterTypeStorage>(layout, binding);

			for (YAML::Node layoutEntry : layoutNode) {
				std::string name = layoutEntry["Name"].as<std::string>();
				ShaderDataType type = StringToShaderDataType(layoutEntry["Type"].as<std::string>());
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
			out << YAML::Key << "TextureHandle" << YAML::Value << sampler2DStorageParameter->GetTextureHandle();

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
			out << YAML::Key << "TextureArrayHandle" << YAML::Value << sampler2DArrayStorageParameter->GetTextureArrayHandle();

			out << YAML::EndMap;
		}
		static Ref<Sampler2DArrayShaderParameterTypeStorage> ReadSampler2DArrayObject(YAML::Node& valueNode)
		{
			uint32_t binding = valueNode["Binding"].as<uint64_t>();
			AssetHandle textureArrayHandle = valueNode["TextureArrayHandle"].as<AssetHandle>();

			return CreateRef<Sampler2DArrayShaderParameterTypeStorage>(textureArrayHandle, binding);
		}
	}

	static std::string ShaderParameterTypeToString(ShaderParameterType type) {
		switch (type)
		{
			case ShaderParameterType::None: return "None";
			case ShaderParameterType::UBO: return "UBO";
			case ShaderParameterType::Sampler2D: return "Sampler2D";
			case ShaderParameterType::Sampler2DArray: return "Sampler2DArray";
		}
		PE_CORE_ASSERT(false, "Unknown shader parameter type!");
		return "";
	}

	static ShaderParameterType StringToShaderParameterType(const std::string& input) {
		if (input == "None") { return ShaderParameterType::None; }
		else if (input == "UBO") { return ShaderParameterType::UBO; }
		else if (input == "Sampler2D") { return ShaderParameterType::Sampler2D; }
		else if (input == "Sampler2DArray") { return ShaderParameterType::Sampler2DArray; }
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

		for (auto& [name, parameter] : material->m_ShaderParameters) {
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
}