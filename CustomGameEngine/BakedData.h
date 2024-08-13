#pragma once
#include <glm/ext/vector_float3.hpp>
#include "ReflectionProbe.h"
#include <vector>
#include <filesystem>
#include <string>
#include <iostream>
#include "stb_image_write.h"
#include <unordered_map>
#include <map>
namespace Engine {
	class BakedData
	{
	public:
		BakedData() {
			cubemapFaceToString[GL_TEXTURE_CUBE_MAP_POSITIVE_X] = "PositiveX";
			cubemapFaceToString[GL_TEXTURE_CUBE_MAP_NEGATIVE_X] = "NegativeX";
			cubemapFaceToString[GL_TEXTURE_CUBE_MAP_POSITIVE_Y] = "PositiveY";
			cubemapFaceToString[GL_TEXTURE_CUBE_MAP_NEGATIVE_Y] = "NegativeY";
			cubemapFaceToString[GL_TEXTURE_CUBE_MAP_POSITIVE_Z] = "PositiveZ";
			cubemapFaceToString[GL_TEXTURE_CUBE_MAP_NEGATIVE_Z] = "NegativeZ";
		}

		~BakedData() {
			ClearReflectionProbes();
		}

		const std::vector<ReflectionProbe*>& GetReflectionProbes() const { return reflectionProbes; }

		void InitialiseReflectionProbes(const std::vector<glm::vec3>& positions, const std::vector<AABBPoints>& localGeometryBounds, const std::vector<float>& soiRadii, const std::vector<unsigned int>& faceRes, const std::vector<float>& nearClips, const std::vector<float>& farClips, const std::vector<bool>& renderSkybox, const std::string& sceneName) {
			ClearReflectionProbes();

			std::string filePath = "Data/ReflectionProbe/" + sceneName;
			std::filesystem::path directoryPath = filePath;

			// Create directory if it doesn't already exist
			if (!std::filesystem::exists(directoryPath)) {
				std::filesystem::create_directory(directoryPath);
				std::cout << "BAKEDDATA::REFLECTIONPROBES::Created directory: " << filePath << std::endl;
			}

			for (int i = 0; i < positions.size(); i++) {
				reflectionProbes.push_back(new ReflectionProbe(i, positions[i], sceneName, localGeometryBounds[i], soiRadii[i], faceRes[i], faceRes[i], nearClips[i], farClips[i], renderSkybox[i]));

				std::string probeString = std::to_string(i);

				// Create file for this reflection probe if it doesn't already exist
				std::filesystem::path probePath = filePath + "/" + probeString;
				if (!std::filesystem::exists(probePath)) {
					std::filesystem::create_directory(probePath);
					std::cout << "BAKEDDATA::REFLECTIONPROBES::Created directory: " << probePath << std::endl;
				}

				// Create files for each cubemap type
				// Skybox
				std::filesystem::path skyboxPath = filePath + "/" + probeString + "/Skybox";
				if (!std::filesystem::exists(skyboxPath)) {
					std::filesystem::create_directory(skyboxPath);
					std::cout << "BAKEDDATA::REFLECTIONPROBES::Created directory: " << skyboxPath << std::endl;
				}

				// Irradiance map
				std::filesystem::path irradiancePath = filePath + "/" + probeString + "/Irradiance";
				if (!std::filesystem::exists(irradiancePath)) {
					std::filesystem::create_directory(irradiancePath);
					std::cout << "BAKEDDATA::REFLECTIONPROBES::Created directory: " << irradiancePath << std::endl;
				}

				// Prefilter map
				std::filesystem::path prefilterPath = filePath + "/" + probeString + "/Prefilter";
				if (!std::filesystem::exists(prefilterPath)) {
					std::filesystem::create_directory(prefilterPath);
					std::cout << "BAKEDDATA::REFLECTIONPROBES::Created directory: " << prefilterPath << std::endl;
				}

				// Prefilter mip levels
				std::string prefilterPathString = prefilterPath.string();
				std::filesystem::path prefilterMipPath;
				for (unsigned int mip = 0; mip < 5; mip++) {
					prefilterMipPath = prefilterPathString + "/Mip " + std::to_string(mip);
					if (!std::filesystem::exists(prefilterMipPath)) {
						std::filesystem::create_directory(prefilterMipPath);
						std::cout << "BAKEDDATA::REFLECTIONPROBES::Created directory: " << prefilterMipPath << std::endl;
					}
				}
			}
		}

		void WriteReflectionProbesToFile();
		void LoadReflectionProbesFromFile();

		void SetCulledProbeList(const std::map<float, ReflectionProbe*>& newProbeList) { this->culledProbeList = newProbeList; }
		const std::map<float, ReflectionProbe*>& GetCulledProbeList() const { return culledProbeList; }
	private:
		void ClearReflectionProbes() {
			for (ReflectionProbe* probe : reflectionProbes) {
				delete probe;
			}
			reflectionProbes.clear();
		}

		std::unordered_map<GLenum, std::string> cubemapFaceToString;

		std::vector<ReflectionProbe*> reflectionProbes;
		std::map<float, ReflectionProbe*> culledProbeList; // <distance to probe squared, probe>
	};
}