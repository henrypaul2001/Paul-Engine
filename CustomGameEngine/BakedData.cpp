#include "BakedData.h"
namespace Engine {
	void BakedData::WriteReflectionProbesToFile()
	{
		std::cout << "BAKEDDATA::Writing reflection probes to file" << std::endl;

		unsigned int numProbes = reflectionProbes.size();
		for (unsigned int i = 0; i < reflectionProbes.size(); i++) {
			ReflectionProbe* probe = reflectionProbes[i];
			std::cout << "        - Writing probe " << i + 1 << " / " << numProbes << std::endl;

			ReflectionProbeEnvironmentMap envMap = probe->GetProbeEnvMap();
			unsigned int probeID = probe->GetFileID();

			// --- Write cubemap ---
			// ---------------------
			
			// -  Write irradiance -
			// ---------------------

			// -- Write prefilter --
			// ---------------------

			// --- Write BRDFLUT ---
			// ---------------------
			stbi_flip_vertically_on_write(true);
			std::string path = "Data/ReflectionProbe/" + probe->GetSceneName() + "/" + std::to_string(probeID) + "/BRDFLut.png";
			
			int width = probe->GetFaceWidth();
			int height = probe->GetFaceHeight();

			// Get texture data as float array
			GLfloat* floatData = new GLfloat[width * height * 2]; // 2 channels (R, G)

			glBindTexture(GL_TEXTURE_2D, envMap.brdf_lutID);
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RG, GL_FLOAT, floatData);

			// Convert to bytes
			unsigned char* byteData = new unsigned char[width * height * 4]; // 4 channels for PNG (R, G, B, A)

			for (unsigned int i = 0; i < width * height; i++) {
				byteData[i * 4] = static_cast<unsigned char>(floatData[i * 2] * 255.0f); // R channel
				byteData[i * 4 + 1] = static_cast<unsigned char>(floatData[i * 2 + 1] * 255.0f); // G channel
				byteData[i * 4 + 2] = 0; // B channel hard set to 0
				byteData[i * 4 + 3] = 255; // A channel hard set to fully opaque
			}

			stbi_write_png(path.c_str(), probe->GetFaceWidth(), probe->GetFaceHeight(), 4, byteData, width * 4);

			delete[] floatData;
			delete[] byteData;
		}
	}
}