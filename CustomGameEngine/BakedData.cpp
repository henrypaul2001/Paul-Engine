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

			// --- Write cubemap ---
			// ---------------------
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

			// -  Write irradiance -
			// ---------------------
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

			// Instead of saving mip level 0, this should save each cubemap face at each mip level
			floatData = new GLfloat[(faceWidth / 2) * (faceHeight / 2) * 3];

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, envMap.prefilterID);
			glFinish();

			for (unsigned int j = 0; j < 6; j++) {
				path = "Data/ReflectionProbe/" + probe->GetSceneName() + "/" + probeString + "/Prefilter/" += cubemapFaceToString[GL_TEXTURE_CUBE_MAP_POSITIVE_X + j] + ".hdr";

				glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, 0, GL_RGB, GL_FLOAT, floatData);
				glFinish();

				stbi_write_hdr(path.c_str(), (faceWidth / 2), (faceHeight / 2), 3, floatData);
			}

			delete[] floatData;

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
}