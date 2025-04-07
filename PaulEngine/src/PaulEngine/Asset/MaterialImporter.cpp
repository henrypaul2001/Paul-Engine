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
		return Ref<Material>();
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
					out << YAML::Value << "Not yet implemented";
					break;
				}
				case ShaderParameterType::Sampler2DArray:
				{
					out << YAML::Value << "Not yet implemented";
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