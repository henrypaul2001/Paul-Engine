#pragma once
#include <glm/ext/vector_float3.hpp>
#include "ReflectionProbe.h"
#include <vector>
#include <filesystem>
#include <string>
#include <iostream>
#include "stb_image_write.h"
namespace Engine {
	class BakedData
	{
	public:
		BakedData() {}
		~BakedData() {
			ClearReflectionProbes();
		}

		const std::vector<ReflectionProbe*>& GetReflectionProbes() const { return reflectionProbes; }

		void InitialiseReflectionProbes(const std::vector<glm::vec3>& positions, const std::string& sceneName) {
			ClearReflectionProbes();

			std::string filePath = "Data/ReflectionProbe/" + sceneName;
			std::filesystem::path directoryPath = filePath;

			// Create directory if it doesn't already exist
			if (!std::filesystem::exists(directoryPath)) {
				std::filesystem::create_directory(directoryPath);
				std::cout << "BAKEDDATA::REFLECTIONPROBES::Created directory: " << filePath << std::endl;
			}

			for (int i = 0; i < positions.size(); i++) {
				reflectionProbes.push_back(new ReflectionProbe(i, positions[i], sceneName));

				// Create file for this reflection probe if it doesn't already exist
				std::filesystem::path probePath = filePath + "/" + std::to_string(i);

				if (!std::filesystem::exists(probePath)) {
					std::filesystem::create_directory(probePath);
					std::cout << "BAKEDDATA::REFLECTIONPROBES::Created directory: " << probePath << std::endl;
				}
			}
		}

		void WriteReflectionProbesToFile();

	private:
		void ClearReflectionProbes() {
			for (ReflectionProbe* probe : reflectionProbes) {
				delete probe;
			}
			reflectionProbes.clear();
		}

		std::vector<ReflectionProbe*> reflectionProbes;
	};
}