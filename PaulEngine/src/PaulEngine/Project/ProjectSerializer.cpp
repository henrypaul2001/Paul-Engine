#include "pepch.h"
#include "ProjectSerializer.h"

#include <fstream>
#include <yaml-cpp/yaml.h>

namespace PaulEngine
{
	ProjectSerializer::ProjectSerializer(Ref<Project> project) : m_Project(project) {}

	bool ProjectSerializer::Serialize(const std::filesystem::path& filepath)
	{
		const ProjectSpecification& spec = m_Project->GetSpecification();

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Project" << YAML::Value;
		
		out << YAML::BeginMap;
		out << YAML::Key << "Name" << YAML::Value << spec.Name;
		out << YAML::Key << "StartScene" << YAML::Value << (uint64_t)spec.StartScene;
		out << YAML::Key << "AssetDirectory" << YAML::Value << spec.AssetDirectory.string();
		out << YAML::Key << "AssetRegistryPath" << YAML::Value << spec.AssetRegistryPath.string();
		out << YAML::EndMap;

		out << YAML::EndMap;

		std::error_code error;
		std::filesystem::create_directories(filepath.parent_path(), error);
		std::ofstream fout = std::ofstream(filepath);
		fout << out.c_str();

		return true;
	}

	bool ProjectSerializer::Deserialize(const std::filesystem::path& filepath)
	{
		ProjectSpecification& spec = m_Project->GetSpecification();

		std::ifstream stream = std::ifstream(filepath);
		std::stringstream ss;
		ss << stream.rdbuf();

		YAML::Node data = YAML::Load(ss.str());
		YAML::Node projectNode = data["Project"];
		if (!projectNode) { return false; }

		spec.Name = projectNode["Name"].as<std::string>();
		spec.StartScene = projectNode["StartScene"].as<uint64_t>();
		spec.AssetDirectory = projectNode["AssetDirectory"].as<std::string>();
		spec.AssetRegistryPath = projectNode["AssetRegistryPath"].as<std::string>();

		return true;
	}
}