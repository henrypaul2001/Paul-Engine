#include "BakedData.h"
#include "Shader.h"
#include "ResourceManager.h"
namespace Engine {
	void BakedData::WriteReflectionProbesToFile()
	{
		std::cout << "BAKEDDATA::Writing reflection probes to file" << std::endl;
		std::string path;
		std::string probeString;

		int faceWidth, faceHeight;

		unsigned int numProbes = reflectionProbes.size();
		for (unsigned int i = 0; i < reflectionProbes.size(); i++) {
			ReflectionProbe* probe = reflectionProbes[i];
			std::cout << "		- Writing probe " << i + 1 << " / " << numProbes << std::endl;

			unsigned int probeID = probe->GetFileID();

			probeString = std::to_string(probeID);

			stbi_flip_vertically_on_write(false);
			faceWidth = probe->GetFaceWidth();
			faceHeight = probe->GetFaceHeight();

			// --- Write skybox ---
			// --------------------
			if (!probe->GetDiscardUnfilteredCapture()) {
				GLfloat* floatData = new GLfloat[faceWidth * faceHeight * 3];

				glBindTexture(GL_TEXTURE_CUBE_MAP, probe->GetCubemapTextureID());
				glFinish();

				for (unsigned int j = 0; j < 6; j++) {
					path = "Data/ReflectionProbe/" + probe->GetSceneName() + "/" + probeString + "/Skybox/" += cubemapFaceToString[GL_TEXTURE_CUBE_MAP_POSITIVE_X + j] + ".hdr";

					glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, 0, GL_RGB, GL_FLOAT, floatData);
					glFinish();

					stbi_write_hdr(path.c_str(), faceWidth, faceHeight, 3, floatData);
				}

				delete[] floatData;
			}

			// - Write irradiance -
			// --------------------
			GLfloat* floatData = new GLfloat[32 * 32 * 3];

			glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, reflectionProbeIrradianceMapArray);
			glFinish();

			unsigned int layer;
			for (unsigned int j = 0; j < 6; j++) {
				path = "Data/ReflectionProbe/" + probe->GetSceneName() + "/" + probeString + "/Irradiance/" += cubemapFaceToString[GL_TEXTURE_CUBE_MAP_POSITIVE_X + j] + ".hdr";

				layer = i * 6 + j;

				glGetTextureSubImage(reflectionProbeIrradianceMapArray, 0, 0, 0, layer, 32, 32, 1, GL_RGB, GL_FLOAT, 32 * 32 * 3 * sizeof(GLfloat), floatData);
				glFinish();

				stbi_write_hdr(path.c_str(), 32, 32, 3, floatData);
			}

			delete[] floatData;

			// -- Write prefilter --
			// ---------------------
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, reflectionProbePrefilterMapArray);
			glFinish();

			std::string mipString;
			unsigned int mipWidth = faceWidth / 2;
			unsigned int mipHeight = faceHeight / 2;
			for (unsigned int mip = 0; mip < 5; mip++) {
				floatData = new GLfloat[mipWidth * mipHeight * 3];
				for (unsigned int j = 0; j < 6; j++) {
					path = "Data/ReflectionProbe/" + probe->GetSceneName() + "/" + probeString + "/Prefilter/Mip " + std::to_string(mip) + "/" + cubemapFaceToString[GL_TEXTURE_CUBE_MAP_POSITIVE_X + j] + ".hdr";

					layer = i * 6 + j;

					glGetTextureSubImage(reflectionProbePrefilterMapArray, mip, 0, 0, layer, mipWidth, mipHeight, 1, GL_RGB, GL_FLOAT, mipWidth * mipHeight * 3 * sizeof(GLfloat), floatData);
					glFinish();

					stbi_write_hdr(path.c_str(), mipWidth, mipHeight, 3, floatData);
				}
				mipWidth /= 2;
				mipHeight /= 2;
				delete[] floatData;
			}
		}
	}

	void BakedData::LoadReflectionProbesFromFile()
	{
		stbi_set_flip_vertically_on_load(true);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		std::cout << "BAKEDDATA::Loading reflection probes from file" << std::endl;

		std::string faces[6] =
		{
			"/" + cubemapFaceToString[GL_TEXTURE_CUBE_MAP_POSITIVE_X + 0] + ".hdr",
			"/" + cubemapFaceToString[GL_TEXTURE_CUBE_MAP_POSITIVE_X + 1] + ".hdr",
			"/" + cubemapFaceToString[GL_TEXTURE_CUBE_MAP_POSITIVE_X + 2] + ".hdr",
			"/" + cubemapFaceToString[GL_TEXTURE_CUBE_MAP_POSITIVE_X + 3] + ".hdr",
			"/" + cubemapFaceToString[GL_TEXTURE_CUBE_MAP_POSITIVE_X + 4] + ".hdr",
			"/" + cubemapFaceToString[GL_TEXTURE_CUBE_MAP_POSITIVE_X + 5] + ".hdr",
		};

		unsigned int numProbes = reflectionProbes.size();
		for (unsigned int i = 0; i < numProbes; i++) {
			std::cout << "		- Loading probe " << i + 1 << " / " << numProbes << std::endl;

			ReflectionProbe* probe = reflectionProbes[i];
			unsigned int probeID = probe->GetFileID();
			unsigned int faceWidth = probe->GetFaceWidth();
			unsigned int faceHeight = probe->GetFaceHeight();
			std::string probeFilepath = "Data/ReflectionProbe/" + probe->GetSceneName() + "/" + std::to_string(probeID);

			// Cubemaps
			// --- Load skybox ---
			// -------------------
			unsigned int skybox = probe->GetCubemapTextureID();
			std::string cubemapFilepath = probeFilepath + "/Skybox";
			FILE* f;
			fopen_s(&f, (cubemapFilepath + faces[0]).c_str(), "rb");
			if (f == NULL) {
				// Discard skybox
				std::cout << "BAKEDDATA::Skybox not saved " << probeID << std::endl;
				glDeleteTextures(1, &skybox);
				probe->SetDiscardUnfilteredCapture(true);
			}
			else {
				fclose(f);
				probe->SetDiscardUnfilteredCapture(false);
				glBindTexture(GL_TEXTURE_CUBE_MAP, skybox);
				std::string filepath = cubemapFilepath;
				int width, height, nrChannels;
				for (unsigned int j = 0; j < 6; j++) {
					filepath = cubemapFilepath + faces[j];

					// Load from file
					float* floatData = stbi_loadf(filepath.c_str(), &width, &height, &nrChannels, 0);

					// Check result
					if (floatData) {
						// Check dimensions
						if (width != faceWidth || height != faceHeight) {
							std::cout << "ERROR::BAKEDDATA::Mismatch of data when loading skybox cubemap for probe " << probeID << std::endl;
							stbi_image_free(floatData);
							break;
						}

						// Read into texture
						glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, 0, GL_RGB16F, faceWidth, faceHeight, 0, GL_RGB, GL_FLOAT, floatData);
					}
					else {
						std::cout << "ERROR::BAKEDDATA::Cubemap face failed to load at path: " << filepath << std::endl;
					}
					stbi_image_free(floatData);
				}
				glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
			}

			// - Load irradiance -
			// -------------------
			std::string irradianceFilepath = probeFilepath + "/Irradiance";
			glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, reflectionProbeIrradianceMapArray);
			std::string filepath = irradianceFilepath;
			unsigned int layer;
			int width, height, nrChannels;
			for (unsigned int j = 0; j < 6; j++) {
				filepath = irradianceFilepath + faces[j];

				// Load from file
				float* floatData = stbi_loadf(filepath.c_str(), &width, &height, &nrChannels, 0);

				// Check result
				if (floatData) {
					// Check dimensions
					if (width != 32 || height != 32) {
						std::cout << "ERROR::BAKEDDATA::Mismatch of data when loading irradiance cubemap for probe " << probeID << std::endl;
						stbi_image_free(floatData);
						break;
					}

					layer = i * 6 + j;

					// Read into texture
					glTexSubImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, 0, 0, layer, 32, 32, 1, GL_RGB, GL_FLOAT, floatData);
				}
				else {
					std::cout << "ERROR::BAKEDDATA::Cubemap face failed to load at path: " << filepath << std::endl;
				}
				stbi_image_free(floatData);
			}
			glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, 0);

			// -- Load prefilter --
			// --------------------
			stbi_set_flip_vertically_on_load(false);
			std::string prefilterFilepath = probeFilepath + "/Prefilter/Mip ";
			glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, reflectionProbePrefilterMapArray);
			unsigned int mipWidth = faceWidth / 2;
			unsigned int mipHeight = faceHeight / 2;
			for (unsigned int mip = 0; mip < 5; mip++) {
				for (unsigned int j = 0; j < 6; j++) {
					filepath = prefilterFilepath + std::to_string(mip) + faces[j];

					// Load from file
					float* floatData = stbi_loadf(filepath.c_str(), &width, &height, &nrChannels, 0);

					// Check result
					if (floatData) {
						// Check dimensions
						if (width != mipWidth || height != mipHeight) {
							std::cout << "ERROR::BAKEDDATA::Mismatch of data when loading prefilter cubemap for probe " << probeID << std::endl;
							stbi_image_free(floatData);
							break;
						}
						
						layer = i * 6 + j;

						// Read into texture
						glTexSubImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, mip, 0, 0, layer, mipWidth, mipHeight, 1, GL_RGB, GL_FLOAT, floatData);
					}
					else {
						std::cout << "ERROR::BAKEDDATA::Cubemap face failed to load at path: " << filepath << std::endl;
					}
					stbi_image_free(floatData);
				}
				mipWidth /= 2;
				mipHeight /= 2;
			}
			glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, 0);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}
}