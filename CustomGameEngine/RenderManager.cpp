#include "RenderManager.h"
#include <glad/glad.h>
#include <cstddef>
#include <iostream>
#include "ForwardPipeline.h"
#include "DeferredPipeline.h"
#include <random>
namespace Engine {
	RenderManager* RenderManager::instance = nullptr;
	RenderManager::RenderManager(unsigned int shadowWidth, unsigned int shadowHeight, unsigned int screenWidth, unsigned int screenHeight)
	{
		renderPipeline = new DeferredPipeline();
		flatDepthMapFBO = new unsigned int;
		cubeDepthMapFBO = new unsigned int;
		texturedFBO = new unsigned int;
		this->screenWidth = screenWidth;
		this->screenHeight = screenHeight;
		SetupShadowMapTextures(shadowWidth, shadowHeight);
		SetupFlatShadowMapFBO();
		SetupCubeShadowMapFBO();
		SetupTexturedFBO(screenWidth, screenHeight);
		SetupGBuffer();
		SetupSSAOBuffers();
		SetupEnvironmentMapFBO();
		SetupCubemapFBO();
		SetupSSR();

		flatShadowmapAtlas = new FlatTextureAtlas(2, 4, shadowWidth, shadowHeight, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

		renderParams = new RenderParams;
		SetupAdvBloom();

		Shader* defaultTextShader = ResourceManager::GetInstance()->DefaultTextShader();
		glm::mat4 textProjection = glm::ortho(0.0f, (float)screenWidth, 0.0f, (float)screenHeight);
		defaultTextShader->Use();
		defaultTextShader->setMat4("projection", textProjection);

		global_brdf_lutID = CreateBRDF();

		ResourceManager::GetInstance()->LoadCubemap("Textures/Cubemaps/Space", ".png", true);
		SetSkyboxTexture("Textures/Cubemaps/Space");
	}

	RenderManager* RenderManager::GetInstance()
	{
		return instance;
	}

	RenderManager* RenderManager::GetInstance(unsigned int shadowWidth, unsigned int shadowHeight, unsigned int screenWidth, unsigned int screenHeight)
	{
		if (instance == nullptr) {
			instance = new RenderManager(shadowWidth, shadowHeight, screenWidth, screenHeight);
		}
		return instance;
	}

	RenderManager::~RenderManager()
	{
		delete renderPipeline;
		delete flatShadowmapAtlas;
		glDeleteTextures(1, &pointShadowArray);

		delete depthMap;
		delete flatDepthMapFBO;
		delete cubeDepthMapFBO;
		delete texturedFBO;
		
		delete gBuffer;
		delete gPosition;
		delete gNormal;
		delete gAlbedo;
		delete gSpecular;
		delete gArm;
		delete gPBRFLAG;
		delete gViewSpacePos;

		delete ssaoFBO;
		delete ssaoBlurFBO;
		delete ssaoColourBuffer;
		delete ssaoBlurColourBuffer;
		delete noiseTexture;
		
		delete screenTexture;
		delete alternateScreenTexture;

		delete bloomBrightnessBuffer;
		delete alternateBloomBrightnessBuffer;

		delete pingPongFBO[0];
		delete pingPongFBO[1];
		delete pingPongColourBuffers[0];
		delete pingPongColourBuffers[1];

		for (glm::vec3* v : ssaoKernel) {
			delete v;
		}

		delete hdrCubeCaptureFBO;
		delete hdrCubeCaptureRBO;
		delete envCubemapTexture;

		for (int i = 0; i < advBloomMipChain.size(); i++) {
			glDeleteTextures(1, &advBloomMipChain[i].texture);
			advBloomMipChain[i].texture = 0;
		}
		glDeleteFramebuffers(1, advBloomFBO);
		delete advBloomFBO;

		delete renderParams;

		glDeleteTextures(1, &global_brdf_lutID);
		glDeleteTextures(1, &ssrUVMap);
		glDeleteTextures(1, &ssrReflectionMap);

		delete instance;
	}

	unsigned int RenderManager::ConvertHDREquirectangularToCube(const unsigned int& textureId)
	{
		glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		glm::mat4 captureViews[] = {
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
		};

		// Cubemap memory allocation
		unsigned int envCubemapTexture;
		glGenTextures(1, &envCubemapTexture);
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemapTexture);
		for (unsigned int i = 0; i < 6; ++i) {
			// store each face with 16 bit floating point values
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		Shader* convertToCubemapShader = ResourceManager::GetInstance()->EquirectangularToCubemapShader();
		convertToCubemapShader->Use();
		convertToCubemapShader->setInt("equirectangularMap", 0);
		convertToCubemapShader->setMat4("projection", captureProjection);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureId);

		glDisable(GL_CULL_FACE);
		glViewport(0, 0, 512, 512);
		glBindFramebuffer(GL_FRAMEBUFFER, *hdrCubeCaptureFBO);
		glBindRenderbuffer(GL_RENDERBUFFER, *hdrCubeCaptureRBO);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		unsigned int depthBuffer = cubemapDepthBuffer;
		// Resize depth texture buffer
		glBindTexture(GL_TEXTURE_CUBE_MAP, depthBuffer);
		for (unsigned int i = 0; i < 6; i++) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT24, 512, 512, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
		}
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, *hdrCubeCaptureFBO);

		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
		for (unsigned int i = 0; i < 6; i++) {
			convertToCubemapShader->setMat4("view", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemapTexture, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, depthBuffer, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			ResourceManager::GetInstance()->DefaultCube().DrawWithNoMaterial();
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, screenWidth, screenHeight);

		return envCubemapTexture;
	}

	unsigned int RenderManager::CreateIrradianceMap()
	{
		unsigned int irradianceMap;
		glGenTextures(1, &irradianceMap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
		for (unsigned int i = 0; i < 6; i++) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindFramebuffer(GL_FRAMEBUFFER, *hdrCubeCaptureFBO);
		glBindRenderbuffer(GL_RENDERBUFFER, *hdrCubeCaptureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

		return irradianceMap;
	}

	void RenderManager::ConvoluteEnvironmentMap(const unsigned int& environmentMap, const unsigned int& irradianceMap)
	{
		glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		glm::mat4 captureViews[] = {
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
		};

		Shader* irradianceShader = ResourceManager::GetInstance()->CreateIrradianceShader();
		irradianceShader->Use();
		irradianceShader->setInt("environmentMap", 0);
		irradianceShader->setMat4("projection", captureProjection);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, environmentMap);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		unsigned int depthBuffer = cubemapDepthBuffer;
		// Resize depth texture buffer
		glBindTexture(GL_TEXTURE_CUBE_MAP, depthBuffer);
		for (unsigned int i = 0; i < 6; i++) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT24, 32, 32, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
		}
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		glViewport(0, 0, 32, 32);
		glBindFramebuffer(GL_FRAMEBUFFER, *hdrCubeCaptureFBO);
		glBindRenderbuffer(GL_RENDERBUFFER, *hdrCubeCaptureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

		glBindTexture(GL_TEXTURE_CUBE_MAP, environmentMap);

		for (unsigned int i = 0; i < 6; ++i)
		{
			irradianceShader->setMat4("view", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, depthBuffer, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			ResourceManager::GetInstance()->DefaultCube().DrawWithNoMaterial();
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, screenWidth, screenHeight);
	}

	unsigned int RenderManager::CreatePrefilterMap(const unsigned int& environmentMap)
	{
		glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		glm::mat4 captureViews[] = {
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
		};

		// Allocate mipmap memory
		// ----------------------
		unsigned int prefilterMap;
		glGenTextures(1, &prefilterMap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
		for (unsigned int i = 0; i < 6; i++) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128 * 2, 128 * 2, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		// Capture prefilter mipmap levels
		// -------------------------------
		Shader* prefilterShader = ResourceManager::GetInstance()->CreatePrefilterShader();
		prefilterShader->Use();
		prefilterShader->setInt("environmentMap", 0);
		prefilterShader->setMat4("projection", captureProjection);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, environmentMap);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		unsigned int depthBuffer = cubemapDepthBuffer;

		glBindFramebuffer(GL_FRAMEBUFFER, *hdrCubeCaptureFBO);
		unsigned int maxMipLevels = 5;
		for (unsigned int mip = 0; mip < maxMipLevels; mip++) {
			// resize framebuffer
			unsigned int mipWidth = (128 * 2) * std::pow(0.5, mip);
			unsigned int mipHeight = (128 * 2) * std::pow(0.5, mip);
			glBindRenderbuffer(GL_RENDERBUFFER, *hdrCubeCaptureRBO);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			// Resize depth texture buffer
			glBindTexture(GL_TEXTURE_CUBE_MAP, depthBuffer);
			for (unsigned int i = 0; i < 6; i++) {
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT24, mipWidth, mipHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
			}
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
			glBindTexture(GL_TEXTURE_CUBE_MAP, environmentMap);
			glBindFramebuffer(GL_FRAMEBUFFER, *hdrCubeCaptureFBO);

			glViewport(0, 0, mipWidth, mipHeight);

			float roughness = (float)mip / (float)(maxMipLevels - 1);
			prefilterShader->setFloat("roughness", roughness);
			prefilterShader->setUInt("faceWidth", 512);
			prefilterShader->setUInt("faceHeight", 512);
			for (unsigned int i = 0; i < 6; i++) {
				prefilterShader->setMat4("view", captureViews[i]);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, depthBuffer, 0);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				ResourceManager::GetInstance()->DefaultCube().DrawWithNoMaterial();
			}
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, screenWidth, screenHeight);

		return prefilterMap;
	}

	unsigned int RenderManager::CreateBRDF()
	{
		unsigned int brdfLUTTexture;
		glGenTextures(1, &brdfLUTTexture);

		glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindFramebuffer(GL_FRAMEBUFFER, *hdrCubeCaptureFBO);
		glBindRenderbuffer(GL_RENDERBUFFER, *hdrCubeCaptureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

		glDisable(GL_CULL_FACE);
		glDisable(GL_BLEND);
		glViewport(0, 0, 512, 512);
		ResourceManager::GetInstance()->CreateBRDFShader()->Use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ResourceManager::GetInstance()->DefaultCube().DrawWithNoMaterial();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, screenWidth, screenHeight);
		glEnable(GL_CULL_FACE);
		
		return brdfLUTTexture;
	}

	void RenderManager::SetupShadowMapTextures(unsigned int shadowWidth, unsigned int shadowHeight)
	{
		this->shadowHeight = shadowHeight;
		this->shadowWidth = shadowWidth;

		// Directional light depth map
		depthMap = new unsigned int;
		glActiveTexture(GL_TEXTURE0);
		glGenTextures(1, depthMap);
		glBindTexture(GL_TEXTURE_2D, *depthMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowWidth, shadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColour[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColour);

		// Set up cubemap array for point shadows
		glGenTextures(1, &pointShadowArray);
		glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, pointShadowArray);
		glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_DEPTH_COMPONENT, shadowWidth, shadowHeight, 8 * 6, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, 0);
	}

	void RenderManager::SetupGBuffer()
	{
		gBuffer = new unsigned int;
		glGenFramebuffers(1, gBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, *gBuffer);

		gPosition = new unsigned int;
		gNormal = new unsigned int;
		gAlbedo = new unsigned int;
		gSpecular = new unsigned int;
		gArm = new unsigned int;
		gPBRFLAG = new unsigned int;
		gViewSpacePos = new unsigned int;

		// position colour buffer
		glGenTextures(1, gPosition);
		glBindTexture(GL_TEXTURE_2D, *gPosition);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *gPosition, 0);
		// normal colour buffer
		glGenTextures(1, gNormal);
		glBindTexture(GL_TEXTURE_2D, *gNormal);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, *gNormal, 0);
		// colour buffer
		glGenTextures(1, gAlbedo);
		glBindTexture(GL_TEXTURE_2D, *gAlbedo);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, *gAlbedo, 0);
		// specular buffer
		glGenTextures(1, gSpecular);
		glBindTexture(GL_TEXTURE_2D, *gSpecular);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, *gSpecular, 0);
		// ao, roughness, metallic buffer (pbr)
		glGenTextures(1, gArm);
		glBindTexture(GL_TEXTURE_2D, *gArm);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, *gArm, 0);
		// PBRFLAG
		glGenTextures(1, gPBRFLAG);
		glBindTexture(GL_TEXTURE_2D, *gPBRFLAG);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, screenWidth, screenHeight, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_2D, *gPBRFLAG, 0);
		// View Space Pos
		glGenTextures(1, gViewSpacePos);
		glBindTexture(GL_TEXTURE_2D, *gViewSpacePos);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT6, GL_TEXTURE_2D, *gViewSpacePos, 0);

		unsigned int attachments[7] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6 };
		glDrawBuffers(7, attachments);

		// create and attach depth buffer
		unsigned int rboDepth;
		glGenRenderbuffers(1, &rboDepth);
		glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screenWidth, screenHeight);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	float lerp(float a, float b, float f)
	{
		return a + f * (b - a);
	}

	void RenderManager::SetupSSAOBuffers()
	{
		ssaoFBO = new unsigned int;
		ssaoBlurFBO = new unsigned int;
		ssaoColourBuffer = new unsigned int;
		ssaoBlurColourBuffer = new unsigned int;
		noiseTexture = new unsigned int;

		glGenFramebuffers(1, ssaoFBO);
		glGenFramebuffers(1, ssaoBlurFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, *ssaoFBO);

		// SSAO colour buffer
		glGenTextures(1, ssaoColourBuffer);
		glBindTexture(GL_TEXTURE_2D, *ssaoColourBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, screenWidth, screenHeight, 0, GL_RED, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *ssaoColourBuffer, 0);

		// Blur stage
		glBindFramebuffer(GL_FRAMEBUFFER, *ssaoBlurFBO);
		glGenTextures(1, ssaoBlurColourBuffer);
		glBindTexture(GL_TEXTURE_2D, *ssaoBlurColourBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, screenWidth, screenHeight, 0, GL_RED, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *ssaoBlurColourBuffer, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Generate sample kernel
		std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0 and 1
		std::default_random_engine generator;
		for (unsigned int i = 0; i < 64; i++) {
			glm::vec3* sample = new glm::vec3(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
			*sample = glm::normalize(*sample);
			*sample *= randomFloats(generator);
			float scale = (float)i / 64.0f;
			
			// scale samples to align with center of kernel
			scale = lerp(0.1f, 1.0f, scale * scale);
			*sample *= scale;
			ssaoKernel.push_back(sample);
		}

		// Generate noise texture
		std::vector<glm::vec3> ssaoNoise;
		for (unsigned int i = 0; i < 16; i++) {
			glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f);
			ssaoNoise.push_back(noise);
		}

		glGenTextures(1, noiseTexture);
		glBindTexture(GL_TEXTURE_2D, *noiseTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	void RenderManager::SetupFlatShadowMapFBO()
	{
		glGenFramebuffers(1, flatDepthMapFBO);

		glBindFramebuffer(GL_FRAMEBUFFER, *flatDepthMapFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *depthMap, 0);

		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void RenderManager::SetupCubeShadowMapFBO() {
		glGenFramebuffers(1, cubeDepthMapFBO);

		glBindFramebuffer(GL_FRAMEBUFFER, *cubeDepthMapFBO);

		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void RenderManager::SetupTexturedFBO(unsigned int screenWidth, unsigned int screenHeight)
	{
		glGenFramebuffers(1, texturedFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, *texturedFBO);

		// Generate screen texture
		screenTexture = new unsigned int;
		glGenTextures(1, screenTexture);
		glBindTexture(GL_TEXTURE_2D, *screenTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// alternate
		alternateScreenTexture = new unsigned int;
		glGenTextures(1, alternateScreenTexture);
		glBindTexture(GL_TEXTURE_2D, *alternateScreenTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Generate bloom texture
		bloomBrightnessBuffer = new unsigned int;
		glGenTextures(1, bloomBrightnessBuffer);
		glBindTexture(GL_TEXTURE_2D, *bloomBrightnessBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // clamp to the edge as the blur filter would otherwise sample repeated texture values
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		// alternate
		alternateBloomBrightnessBuffer = new unsigned int;
		glGenTextures(1, alternateBloomBrightnessBuffer);
		glBindTexture(GL_TEXTURE_2D, *alternateBloomBrightnessBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // clamp to the edge as the blur filter would otherwise sample repeated texture values
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		// Attach textures to fbo
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *screenTexture, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, *bloomBrightnessBuffer, 0);

		// Generate render buffer object for depth and stencil testing that won't be sampled
		unsigned int rbo;
		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screenWidth, screenHeight);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

		unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, attachments);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		SetupBloomPingPongFBO();
	}

	void RenderManager::SetupBloomPingPongFBO()
	{
		pingPongFBO[0] = new unsigned int;
		pingPongFBO[1] = new unsigned int;
		pingPongColourBuffers[0] = new unsigned int;
		pingPongColourBuffers[1] = new unsigned int;

		glGenFramebuffers(1, pingPongFBO[0]);
		glGenFramebuffers(1, pingPongFBO[1]);
		glGenTextures(1, pingPongColourBuffers[0]);
		glGenTextures(1, pingPongColourBuffers[1]);
		for (unsigned int i = 0; i < 2; i++) {
			glBindFramebuffer(GL_FRAMEBUFFER, *pingPongFBO[i]);
			glBindTexture(GL_TEXTURE_2D, *pingPongColourBuffers[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // clamp to the edge as the blur filter would otherwise sample repeated texture values
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *pingPongColourBuffers[i], 0);
		}
	}
	
	void RenderManager::SetupEnvironmentMapFBO()
	{
		hdrCubeCaptureFBO = new unsigned int;
		hdrCubeCaptureRBO = new unsigned int;

		// Setup capture framebuffer
		glGenFramebuffers(1, hdrCubeCaptureFBO);
		glGenRenderbuffers(1, hdrCubeCaptureRBO);
		glBindFramebuffer(GL_FRAMEBUFFER, *hdrCubeCaptureFBO);
		glBindRenderbuffer(GL_RENDERBUFFER, *hdrCubeCaptureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, *hdrCubeCaptureRBO);
	}

	void RenderManager::ResizeAdvBloomMipChain(int newSize)
	{
		std::cout << "NOT YET IMPLEMENTED. I'M SHOUTING!" << std::endl;
	}

	void RenderManager::SetupAdvBloom()
	{
		advBloomFBO = new unsigned int;
		glGenFramebuffers(1, advBloomFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, *advBloomFBO);

		glm::vec2 mipSize((float)screenWidth, (float)screenHeight);
		glm::ivec2 mipIntSize((int)screenWidth, (int)screenHeight);

		// Setup mip chain for down/up sampling
		int chainLength = renderParams->GetAdvBloomChainLength();
		advBloomMipChain.reserve(chainLength);
		for (unsigned int i = 0; i < chainLength; i++) {
			AdvBloomMip mip;
			mipSize *= 0.5f;
			mipIntSize /= 2;
			mip.size = mipSize;
			mip.intSize = mipIntSize;

			glGenTextures(1, &mip.texture);
			glBindTexture(GL_TEXTURE_2D, mip.texture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, (int)mipSize.x, (int)mipSize.y, 0, GL_RGB, GL_FLOAT, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			advBloomMipChain.emplace_back(mip);
		}

		// Set up fbo attachment
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, advBloomMipChain[0].texture, 0);
		unsigned int attachments[] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, attachments);

		// error check
		int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			std::cout << "ERROR::RENDERMANAGER::Advanced bloom FBO incomplete, status: 0x\%x\n" << status << std::endl;
			delete advBloomFBO;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void RenderManager::SetupCubemapFBO()
	{
		glGenFramebuffers(1, &cubemapFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, cubemapFBO);

		glGenTextures(1, &cubemapDepthBuffer);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapDepthBuffer);
		for (unsigned int i = 0; i < 6; i++) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT24, 512, 512, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, cubemapDepthBuffer, 0);

		// error check
		int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			std::cout << "ERROR::RENDERMANAGER::Cubemap FBO incomplete, status: 0x\%x\n" << status << std::endl;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void RenderManager::SetupSSR()
	{
		ssrUVMap = 0;
		glGenTextures(1, &ssrUVMap);
		glBindTexture(GL_TEXTURE_2D, ssrUVMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		ssrReflectionMap = 0;
		glGenTextures(1, &ssrReflectionMap);
		glBindTexture(GL_TEXTURE_2D, ssrReflectionMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}