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
			for (auto& [name, data] : ubo->GetLayoutStorage()) {
				out << YAML::BeginMap;

				ShaderDataType type = data->GetType();
				out << YAML::Key << "Type" << YAML::Value << ShaderDataTypeToString(type);
				out << YAML::Key << "Name" << YAML::Value << name;
				out << YAML::Key << "Value" << YAML::Value;

				switch (type)
				{
					case ShaderDataType::None:
						out << "NULL";
						break;
					case ShaderDataType::Float:
						out << *(float*)data->GetData();
						break;
					case ShaderDataType::Float2:
						out << *(glm::vec2*)data->GetData();
						break;
					case ShaderDataType::Float3:
						out << *(glm::vec3*)data->GetData();
						break;
					case ShaderDataType::Float4:
						out << *(glm::vec4*)data->GetData();
						break;
					case ShaderDataType::Mat3:
						out << *(glm::mat3*)data->GetData();
						break;
					case ShaderDataType::Mat4:
						out << *(glm::mat4*)data->GetData();
						break;
					case ShaderDataType::Int:
						out << *(int*)data->GetData();
						break;
					case ShaderDataType::Int2:
						out << *(glm::ivec2*)data->GetData();
						break;
					case ShaderDataType::Int3:
						out << *(glm::ivec3*)data->GetData();
						break;
					case ShaderDataType::Int4:
						out << *(glm::ivec4*)data->GetData();
						break;
					case ShaderDataType::Bool:
						out << *(bool*)data->GetData();
						break;
				}

				out << YAML::EndMap;
			}
			out << YAML::EndSeq;

			out << YAML::EndMap;
		}
		static Ref<UBOShaderParameterTypeStorage> ReadUniformBufferStorageObject(YAML::Node& valueNode)
		{
			uint32_t binding = valueNode["Binding"].as<uint64_t>();
			size_t size = 0;

			std::vector<UniformBufferStorage::LayoutElement> layout;

			YAML::Node layoutNode = valueNode["Layout"];
			for (YAML::Node layoutEntry : layoutNode) {
				ShaderDataType type = StringToShaderDataType(layoutEntry["Type"].as<std::string>());
				std::string name = layoutEntry["Name"].as<std::string>();
				size += ShaderDataTypeSize(type);

				switch (type)
				{
					case ShaderDataType::None:
						continue;
						break;
					case ShaderDataType::Float:
					{
						float* data = new float(layoutEntry["Value"].as<float>());
						layout.push_back({ name, CreateRef<ShaderDataTypeStorage<float>>(type, data) });
						break;
					}
					case ShaderDataType::Float2:
					{
						glm::vec2* data = new glm::vec2(layoutEntry["Value"].as<glm::vec2>());
						layout.push_back({ name, CreateRef<ShaderDataTypeStorage<glm::vec2>>(type, data) });
						break;
					}
					case ShaderDataType::Float3:
					{
						glm::vec3* data = new glm::vec3(layoutEntry["Value"].as<glm::vec3>());
						layout.push_back({ name, CreateRef<ShaderDataTypeStorage<glm::vec3>>(type, data) });
						break;
					}
					case ShaderDataType::Float4:
					{
						glm::vec4* data = new glm::vec4(layoutEntry["Value"].as<glm::vec4>());
						layout.push_back({ name, CreateRef<ShaderDataTypeStorage<glm::vec4>>(type, data) });
						break;
					}
					case ShaderDataType::Mat3:
					{
						glm::mat3* data = new glm::mat3(layoutEntry["Value"].as<glm::mat3>());
						layout.push_back({ name, CreateRef<ShaderDataTypeStorage<glm::mat3>>(type, data) });
						break;
					}
					case ShaderDataType::Mat4:
					{
						glm::mat4* data = new glm::mat4(layoutEntry["Value"].as<glm::mat4>());
						layout.push_back({ name, CreateRef<ShaderDataTypeStorage<glm::mat4>>(type, data) });
						break;
					}
					case ShaderDataType::Int:
					{
						int* data = new int(layoutEntry["Value"].as<int>());
						layout.push_back({ name, CreateRef<ShaderDataTypeStorage<int>>(type, data) });
						break;
					}
					case ShaderDataType::Int2:
					{
						glm::ivec2* data = new glm::ivec2(layoutEntry["Value"].as<glm::ivec2>());
						layout.push_back({ name, CreateRef<ShaderDataTypeStorage<glm::ivec2>>(type, data) });
						break;
					}
					case ShaderDataType::Int3:
					{
						glm::ivec3* data = new glm::ivec3(layoutEntry["Value"].as<glm::ivec3>());
						layout.push_back({ name, CreateRef<ShaderDataTypeStorage<glm::ivec3>>(type, data) });
						break;
					}
					case ShaderDataType::Int4:
					{
						glm::ivec4* data = new glm::ivec4(layoutEntry["Value"].as<glm::ivec4>());
						layout.push_back({ name, CreateRef<ShaderDataTypeStorage<glm::ivec4>>(type, data) });
						break;
					}
					case ShaderDataType::Bool:
					{
						bool* data = new bool(layoutEntry["Value"].as<bool>());
						layout.push_back({ name, CreateRef<ShaderDataTypeStorage<bool>>(type, data) });
						break;
					}
				}
			}

			Ref<UBOShaderParameterTypeStorage> ubo = CreateRef<UBOShaderParameterTypeStorage>(size, binding);
			for (auto& [name, data] : layout) {
				ubo->UBO()->AddDataType(name, data);
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
					material.AddParameterType(paramName, MaterialImporterUtils::ReadUniformBufferStorageObject(value));
					break;
				case ShaderParameterType::Sampler2D:
					material.AddParameterType(paramName, MaterialImporterUtils::ReadSampler2DObject(value));
					break;
				case ShaderParameterType::Sampler2DArray:
					material.AddParameterType(paramName, MaterialImporterUtils::ReadSampler2DArrayObject(value));
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