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

			ReflectionProbeEnvironmentMap envMap = probe->GetProbeEnvMap();
			unsigned int probeID = probe->GetFileID();

			probeString = std::to_string(probeID);

			// --- Write skybox ---
			// --------------------
			stbi_flip_vertically_on_write(false);

			faceWidth = probe->GetFaceWidth();
			faceHeight = probe->GetFaceHeight();
			GLfloat* floatData = new GLfloat[faceWidth * faceHeight * 3];

			glBindTexture(GL_TEXTURE_CUBE_MAP, envMap.cubemapID);
			glFinish();

			for (unsigned int j = 0; j < 6; j++) {
				path = "Data/ReflectionProbe/" + probe->GetSceneName() + "/" + probeString + "/Skybox/" += cubemapFaceToString[GL_TEXTURE_CUBE_MAP_POSITIVE_X + j] + ".hdr";

				glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, 0, GL_RGB, GL_FLOAT, floatData);
				glFinish();

				stbi_write_hdr(path.c_str(), faceWidth, faceHeight, 3, floatData);
			}

			delete[] floatData;

			// - Write irradiance -
			// --------------------
			floatData = new GLfloat[32 * 32 * 3];

			glBindTexture(GL_TEXTURE_CUBE_MAP, envMap.irradianceID);
			glFinish();

			for (unsigned int j = 0; j < 6; j++) {
				path = "Data/ReflectionProbe/" + probe->GetSceneName() + "/" + probeString + "/Irradiance/" += cubemapFaceToString[GL_TEXTURE_CUBE_MAP_POSITIVE_X + j] + ".hdr";

				glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, 0, GL_RGB, GL_FLOAT, floatData);
				glFinish();

				stbi_write_hdr(path.c_str(), 32, 32, 3, floatData);
			}

			delete[] floatData;

			// -- Write prefilter --
			// ---------------------
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, envMap.prefilterID);
			glFinish();

			std::string mipString;
			unsigned int mipWidth = faceWidth / 2;
			unsigned int mipHeight = faceHeight / 2;
			for (unsigned int mip = 0; mip < 5; mip++) {
				floatData = new GLfloat[mipWidth * mipHeight * 3];
				for (unsigned int j = 0; j < 6; j++) {
					path = "Data/ReflectionProbe/" + probe->GetSceneName() + "/" + probeString + "/Prefilter/Mip " + std::to_string(mip) + "/" + cubemapFaceToString[GL_TEXTURE_CUBE_MAP_POSITIVE_X + j] + ".hdr";

					glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, mip, GL_RGB, GL_FLOAT, floatData);
					glFinish();

					stbi_write_hdr(path.c_str(), mipWidth, mipHeight, 3, floatData);
				}
				mipWidth /= 2;
				mipHeight /= 2;
				delete[] floatData;
			}

			// --- Write BRDFLUT ---
			// ---------------------
			stbi_flip_vertically_on_write(true);
			path = "Data/ReflectionProbe/" + probe->GetSceneName() + "/" + probeString + "/BRDFLut.png";

			// Get texture data as float array
			floatData = new GLfloat[faceWidth * faceHeight * 2]; // 2 channels (R, G)

			glBindTexture(GL_TEXTURE_2D, envMap.brdf_lutID);
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RG, GL_FLOAT, floatData);

			// Convert to bytes
			unsigned char* byteData = new unsigned char[faceWidth * faceHeight * 4]; // 4 channels for PNG (R, G, B, A)

			for (unsigned int j = 0; j < faceWidth * faceHeight; j++) {
				byteData[j * 4] = static_cast<unsigned char>(floatData[j * 2] * 255.0f); // R channel
				byteData[j * 4 + 1] = static_cast<unsigned char>(floatData[j * 2 + 1] * 255.0f); // G channel
				byteData[j * 4 + 2] = 0; // B channel hard set to 0
				byteData[j * 4 + 3] = 255; // A channel hard set to fully opaque
			}

			stbi_write_png(path.c_str(), probe->GetFaceWidth(), probe->GetFaceHeight(), 4, byteData, faceWidth * 4);

			delete[] floatData;
			delete[] byteData;
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
			ReflectionProbeEnvironmentMap envMap = probe->GetProbeEnvMap();
			std::string probeFilepath = "Data/ReflectionProbe/" + probe->GetSceneName() + "/" + std::to_string(probeID);

			// Cubemaps
			// --- Load skybox ---
			// -------------------
			std::string cubemapFilepath = probeFilepath + "/Skybox";
			unsigned int skybox = envMap.cubemapID;
			glBindTexture(GL_TEXTURE_CUBE_MAP, skybox);
			std::string filepath = cubemapFilepath;
			int width, height, nrChannels;
			for (unsigned int j = 0; j < 6; j++) {
				filepath = cubemapFilepath + faces[j];

				// Load from file
				float* floatData = stbi_loadf(filepath.c_str(), &width, &height, &nrChannels, 0);

				// Check output
				if (width != faceWidth || height != faceHeight) {
					std::cout << "ERROR::BAKEDDATA::Mismatch of data when loading skybox cubemap for probe " << probeID << std::endl;
					stbi_image_free(floatData);
					break;
				}

				// Read into texture
				if (floatData) {
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, 0, GL_RGB16F, faceWidth, faceHeight, 0, GL_RGB, GL_FLOAT, floatData);
				}
				else {
					std::cout << "ERROR::BAKEDDATA::Cubemap face failed to load at path: " << filepath << std::endl;
				}
				stbi_image_free(floatData);
			}
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

			// - Load irradiance -
			// -------------------
			std::string irradianceFilepath = probeFilepath + "/Irradiance";
			unsigned int irradiance = envMap.irradianceID;
			glBindTexture(GL_TEXTURE_CUBE_MAP, irradiance);
			filepath = irradianceFilepath;
			for (unsigned int j = 0; j < 6; j++) {
				filepath = irradianceFilepath + faces[j];

				// Load from file
				float* floatData = stbi_loadf(filepath.c_str(), &width, &height, &nrChannels, 0);

				// Check output
				if (width != 32 || height != 32) {
					std::cout << "ERROR::BAKEDDATA::Mismatch of data when loading irradiance cubemap for probe " << probeID << std::endl;
					stbi_image_free(floatData);
					break;
				}

				// Read into texture
				if (floatData) {
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, floatData);
				}
				else {
					std::cout << "ERROR::BAKEDDATA::Cubemap face failed to load at path: " << filepath << std::endl;
				}
				stbi_image_free(floatData);
			}
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

			// BRDFLUT
			// Load texture
			//unsigned int textureID;
			//glGenTextures(1, &textureID);

			//int width, height, nrComponents;
			//unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &nrComponents, 0);
			//if (data) {
			//	GLenum internalFormat = GL_RGB;
			//	GLenum dataFormat = GL_RGB;
			//	if (nrComponents == 1) {
			//		internalFormat = GL_RED;
			//		dataFormat = GL_RED;
			//	}
			//	else if (nrComponents == 2) {
			//		internalFormat = GL_RG;
			//		dataFormat = GL_RG;
			//	}
			//	else if (nrComponents == 3) {
			//		internalFormat = srgb ? GL_SRGB : GL_RGB;
			//		dataFormat = GL_RGB;
			//	}
			//	else if (nrComponents == 4) {
			//		internalFormat = srgb ? GL_SRGB_ALPHA : GL_RGBA;
			//		dataFormat = GL_RGBA;
			//	}
			//	glBindTexture(GL_TEXTURE_2D, textureID);
			//	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
			//	glGenerateMipmap(GL_TEXTURE_2D);

			//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			//	stbi_image_free(data);
			//}
			//else {
			//	std::cout << "ERROR::RESOURCEMANAGER::TEXTURELOAD::Texture failed to load at path: " << filepath << std::endl;
			//	stbi_image_free(data);
			//}
		}
	}
}