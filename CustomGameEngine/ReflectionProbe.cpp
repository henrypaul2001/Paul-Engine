#include "ReflectionProbe.h"
#include "RenderManager.h"
namespace Engine {
	ReflectionProbe::ReflectionProbe(unsigned int id, const glm::vec3& position, const unsigned int faceResWidth, const unsigned int faceResHeight, float nearClip, float farClip)
	{
		this->fileID = id;
		this->worldPosition = position;
		this->faceHeight = faceResHeight;
		this->faceWidth = faceResWidth;
		this->nearClip = nearClip;
		this->farClip = farClip;

		SetupTextureMaps();
	}

	ReflectionProbe::~ReflectionProbe()
	{
		glDeleteTextures(1, &envMap.cubemapID);
	}

	void ReflectionProbe::SetupTextureMaps()
	{
		// ------ Set up base cubemap ------
		// ---------------------------------
		glGenTextures(1, &envMap.cubemapID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, envMap.cubemapID);

		for (unsigned int i = 0; i < 6; i++) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, faceWidth, faceHeight, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		// ----- Set up irradiance map -----
		// ---------------------------------
		glGenTextures(1, &envMap.irradianceID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, envMap.irradianceID);
		for (unsigned int i = 0; i < 6; i++) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// ----- Set up pre-filter map -----
		// ---------------------------------
		glGenTextures(1, &envMap.prefilterID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, envMap.prefilterID);
		for (unsigned int i = 0; i < 6; i++) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128 * 2, 128 * 2, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		// ------ Set up brdf_lut map ------
		// ---------------------------------
		glGenTextures(1, &envMap.brdf_lutID);

		glBindTexture(GL_TEXTURE_2D, envMap.brdf_lutID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, faceWidth, faceHeight, 0, GL_RG, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	void ReflectionProbe::LoadBakedReflectionProbe()
	{
		unsigned int environmentMap;
		unsigned int irradianceMap;
		unsigned int prefilterMap;
		unsigned int brdf_LUT;

		//RenderManager* renderInstance = RenderManager::GetInstance();
		//environmentMap = loadFromFile;
		//irradianceMap = renderInstance->CreateIrradianceMap();
		//renderInstance->ConvoluteEnvironmentMap(environmentMap, irradianceMap);
		//prefilterMap = renderInstance->CreatePrefilterMap(environmentMap);
		//brdf_LUT = renderInstance->CreateBRDF();

		//HDREnvironment* cubemap = new HDREnvironment();
		//cubemap->cubemapID = environmentMap;
		//cubemap->irradianceID = irradianceMap;
		//cubemap->brdf_lutID = brdf_LUT;
		//cubemap->prefilterID = prefilterMap;
		//cubemap->filepath = filepath;
	}
}