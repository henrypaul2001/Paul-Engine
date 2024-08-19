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
			glDeleteTextures(1, &reflectionProbeIrradianceMapArray);
			glDeleteTextures(1, &reflectionProbePrefilterMapArray);
			ClearReflectionProbes();
		}

		const std::vector<ReflectionProbe*>& GetReflectionProbes() const { return reflectionProbes; }

		void InitialiseReflectionProbes(const std::vector<glm::vec3>& positions, const std::vector<AABBPoints>& localGeometryBounds, const std::vector<float>& soiRadii, const std::vector<float>& nearClips, const std::vector<float>& farClips, const std::vector<bool>& renderSkybox, const std::string& sceneName, const unsigned int faceRes = 512u) {
			ClearReflectionProbes();

			unsigned int numProbes = positions.size();

			// ----- Set up irradiance map -----
			// ---------------------------------
			glGenTextures(1, &reflectionProbeIrradianceMapArray);
			glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, reflectionProbeIrradianceMapArray);
			glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_RGB16F, 32u, 32u, numProbes * 6, 0, GL_RGB, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, 0);

			// ----- Set up pre-filter map -----
			// ---------------------------------
			glGenTextures(1, &reflectionProbePrefilterMapArray);
			glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, reflectionProbePrefilterMapArray);
			glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_RGB16F, faceRes / 2u, faceRes / 2u, numProbes * 6, 0, GL_RGB, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP_ARRAY);

			for (int i = 0; i < positions.size(); i++) {
				InitialiseReflectionProbe(positions[i], localGeometryBounds[i], sceneName, soiRadii[i], faceRes, nearClips[i], farClips[i], renderSkybox[i]);
			}
		}

		void WriteReflectionProbesToFile();
		void LoadReflectionProbesFromFile();

		void SetCulledProbeList(const std::map<float, ReflectionProbe*>& newProbeList) { this->culledProbeList = newProbeList; }
		const std::map<float, ReflectionProbe*>& GetCulledProbeList() const { return culledProbeList; }

		void ClearBakedData() {
			ClearReflectionProbes();
		}

		const unsigned int GetProbeIrradianceMapArray() const { return reflectionProbeIrradianceMapArray; }
		const unsigned int GetProbePrefilterMapArray() const { return reflectionProbePrefilterMapArray; }
	private:
		// Reflection probes
		// -----------------
		void ClearReflectionProbes() {
			culledProbeList.clear();

			for (ReflectionProbe* probe : reflectionProbes) {
				delete probe;
			}
			reflectionProbes.clear();
		}
		void InitialiseProbeDirectory(const std::string& sceneName, const unsigned int probeIndex) {
			std::string filePath = "Data/ReflectionProbe/" + sceneName;
			std::filesystem::path directoryPath = filePath;

			// Create directory if it doesn't already exist
			if (!std::filesystem::exists(directoryPath)) {
				std::filesystem::create_directory(directoryPath);
				std::cout << "BAKEDDATA::REFLECTIONPROBES::Created directory: " << filePath << std::endl;
			}
			std::string probeString = std::to_string(probeIndex);

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
		void InitialiseReflectionProbe(const glm::vec3& position, const AABBPoints& localGeometryBounds, const std::string& sceneName, const float sphereOfInfluenceRadius, const unsigned int faceResolution, const float nearClip, const float farClip, const bool renderSkybox) {
			unsigned int index = reflectionProbes.size();
			InitialiseProbeDirectory(sceneName, index);
			reflectionProbes.push_back(new ReflectionProbe(index, position, sceneName, localGeometryBounds, sphereOfInfluenceRadius, faceResolution, faceResolution, nearClip, farClip, renderSkybox));
		}

		std::unordered_map<GLenum, std::string> cubemapFaceToString;

		unsigned int reflectionProbeIrradianceMapArray;
		unsigned int reflectionProbePrefilterMapArray;

		std::vector<ReflectionProbe*> reflectionProbes;
		std::map<float, ReflectionProbe*> culledProbeList; // <distance to probe squared, probe>
	};
}