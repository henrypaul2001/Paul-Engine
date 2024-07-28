#include "BakedData.h"
#include "Shader.h"
#include "ResourceManager.h"
namespace Engine {
	void BakedData::WriteReflectionProbesToFile()
	{
		std::cout << "BAKEDDATA::Writing reflection probes to file" << std::endl;
		std::string path;
		std::string probeString;

		GLuint framebuffer;
		glGenFramebuffers(1, &framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

		int faceWidth = 1280;
		int faceHeight = 1280;

		GLuint face0;
		glGenTextures(1, &face0);
		glBindTexture(GL_TEXTURE_2D, face0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, faceWidth, faceHeight, 0, GL_RGB, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		GLuint face1;
		glGenTextures(1, &face1);
		glBindTexture(GL_TEXTURE_2D, face1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, faceWidth, faceHeight, 0, GL_RGB, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		GLuint face2;
		glGenTextures(1, &face2);
		glBindTexture(GL_TEXTURE_2D, face2);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, faceWidth, faceHeight, 0, GL_RGB, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		GLuint face3;
		glGenTextures(1, &face3);
		glBindTexture(GL_TEXTURE_2D, face3);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, faceWidth, faceHeight, 0, GL_RGB, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		GLuint face4;
		glGenTextures(1, &face4);
		glBindTexture(GL_TEXTURE_2D, face4);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, faceWidth, faceHeight, 0, GL_RGB, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		GLuint face5;
		glGenTextures(1, &face5);
		glBindTexture(GL_TEXTURE_2D, face5);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, faceWidth, faceHeight, 0, GL_RGB, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		std::vector<GLuint> faces = { face0, face1, face2, face3, face4, face5 };

		unsigned int numProbes = reflectionProbes.size();
		for (unsigned int i = 0; i < reflectionProbes.size(); i++) {
			ReflectionProbe* probe = reflectionProbes[i];
			std::cout << "        - Writing probe " << i + 1 << " / " << numProbes << std::endl;

			ReflectionProbeEnvironmentMap envMap = probe->GetProbeEnvMap();
			unsigned int probeID = probe->GetFileID();

			probeString = std::to_string(probeID);

			// --- Write cubemap ---
			// ---------------------
			stbi_flip_vertically_on_write(false);
			path = "Data/ReflectionProbe/" + probe->GetSceneName() + "/" + probeString + "/Skybox";

			glDisable(GL_CULL_FACE);

			Shader cubeTo2D = Shader("Shaders/renderCubemapTo2D.vert", "Shaders/renderCubemapTo2D.frag");
			cubeTo2D.Use();
			cubeTo2D.setInt("cubemap", 0);

			glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

			// bind cubemap
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, envMap.cubemapID);

			GLfloat* floatData = new GLfloat[faceWidth * faceHeight * 3]; // 3 channels (R, G, B)

			glViewport(0, 0, faceWidth, faceHeight);
			for (int j = 0; j < 6; j++) {
				cubeTo2D.setInt("faceIndex", j);

				// Bind texture to framebuffer output
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, faces[j], 0);
				
				glDrawBuffer(GL_COLOR_ATTACHMENT0);
				glReadBuffer(GL_COLOR_ATTACHMENT0);

				//glClear(GL_COLOR_BUFFER_BIT);

				// Draw
				ResourceManager::GetInstance()->DefaultPlane().DrawWithNoMaterial();

				glReadPixels(0, 0, faceWidth, faceHeight, GL_RGB, GL_FLOAT, floatData);

				// Debugging: Print the first pixel of each face
				std::cout << "Face " << j << " first pixel RGB: "
					<< floatData[0] << ", "
					<< floatData[1] << ", "
					<< floatData[2] << std::endl;
			}

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
			//glDrawBuffer(GL_NONE);
			
			//floatData = new GLfloat[faceWidth * faceHeight * 3]; // 3 channels (R, G, B)

			//glActiveTexture(GL_TEXTURE0);
			//glBindTexture(GL_TEXTURE_2D, faces[0]);
			//glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, floatData);

			//// Debugging: Print the first pixel of each face
			//std::cout << "Face " << 0 << " first pixel RGB: "
			//	<< floatData[0] << ", "
			//	<< floatData[1] << ", "
			//	<< floatData[2] << std::endl;

			//floatData = new GLfloat[faceWidth * faceHeight * 3];

			//glActiveTexture(GL_TEXTURE1);
			//glBindTexture(GL_TEXTURE_2D, faces[1]);
			//glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, floatData);

			//// Debugging: Print the first pixel of each face
			//std::cout << "Face " << 1 << " first pixel RGB: "
			//	<< floatData[0] << ", "
			//	<< floatData[1] << ", "
			//	<< floatData[2] << std::endl;

			//glActiveTexture(GL_TEXTURE0);

			/*
			float* floatDataCopy = new float[faceWidth * faceHeight * 3];
			for (int j = 0; j < 6; j++) {
				GLfloat* floatDataNew = new GLfloat[faceWidth * faceHeight * 3];

				//path = "Data/ReflectionProbe/" + probe->GetSceneName() + "/" + probeString + "/Skybox/" + cubemapFaceToString[GL_TEXTURE_CUBE_MAP_POSITIVE_X + j] + ".hdr";

				//glBindTexture(GL_TEXTURE_2D, faces[j]);
				//glFinish();

				GLenum err = glGetError();
				if (err != GL_NO_ERROR) {
					std::cerr << "OpenGL error" << ": " << err << std::endl;
				}

				glBindTexture(GL_TEXTURE_2D, faces[j]);
				glGenerateMipmap(GL_TEXTURE_2D);
				glFinish();
				glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, floatDataNew);
				glBindTexture(GL_TEXTURE_2D, 0);
				glFinish();

				err = glGetError();
				if (err != GL_NO_ERROR) {
						std::cerr << "OpenGL error" << ": " << err << std::endl;
				}

				// Cast to float
				//for (unsigned int k = 0; k < faceWidth * faceHeight; k++) {
				//	floatDataCopy[k * 3] = floatDataNew[k * 3]; // R channel
				//	floatDataCopy[k * 3 + 1] = floatDataNew[k * 3 + 1]; // G channel
				//	floatDataCopy[k * 3 + 2] = floatDataNew[k * 3 + 2]; // B channel
				//}

				// Debugging: Print the first pixel of each face
				std::cout << "Face " << j << " first pixel RGB: "
					<< floatDataNew[0] << ", "
					<< floatDataNew[1] << ", "
					<< floatDataNew[2] << std::endl;

				//stbi_write_hdr(path.c_str(), faceWidth, faceHeight, 3, floatDataCopy);
				delete[] floatDataNew;
			}

			delete[] floatDataCopy;
			*/
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

			// -  Write irradiance -
			// ---------------------

			// -- Write prefilter --
			// ---------------------

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

		glDeleteFramebuffers(1, &framebuffer);

		glDeleteTextures(1, &face0);
		glDeleteTextures(1, &face1);
		glDeleteTextures(1, &face2);
		glDeleteTextures(1, &face3);
		glDeleteTextures(1, &face4);
		glDeleteTextures(1, &face5);
	}
}