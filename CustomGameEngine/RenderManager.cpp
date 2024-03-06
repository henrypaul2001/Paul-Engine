#include "RenderManager.h"
#include <glad/glad.h>
#include <cstddef>
namespace Engine {
	RenderManager* RenderManager::instance = nullptr;
	RenderManager::RenderManager(unsigned int shadowWidth, unsigned int shadowHeight, unsigned int screenWidth, unsigned int screenHeight)
	{
		flatDepthMapFBO = new unsigned int;
		cubeDepthMapFBO = new unsigned int;
		texturedFBO = new unsigned int;
		SetupShadowMapTextures(shadowWidth, shadowHeight);
		SetupFlatShadowMapFBO();
		SetupCubeShadowMapFBO();
		SetupTexturedFBO(screenWidth, screenHeight);
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
		delete depthMap;
		delete flatDepthMapFBO;
		delete cubeDepthMapFBO;
		delete texturedFBO;

		for (int i = 0; i < 8; i++) {
			delete flatDepthMaps[i];
			delete cubeDepthMaps[i];
		}

		delete instance;
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

		// Solution
		// use hashmap for textures
		// key = light index, 8 maximum
		// value = unsigned int, texture, can be either cube or 2d

		// Generate 2D textures for spot lights
		for (int i = 0; i < 8; i++) {
			flatDepthMaps.push_back(new unsigned int);

			glActiveTexture(GL_TEXTURE0 + i + 1);
			glGenTextures(1, flatDepthMaps[i]);
			glBindTexture(GL_TEXTURE_2D, *flatDepthMaps[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowWidth, shadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			float borderColour[] = { 1.0f, 1.0f, 1.0f, 1.0f };
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColour);
		}

		// Generate cube map textures for point lights
		for (int i = 0; i < 8; i++) {
			cubeDepthMaps.push_back(new unsigned int);

			glActiveTexture(GL_TEXTURE0 + i + 9);
			glGenTextures(1, cubeDepthMaps[i]);
			glBindTexture(GL_TEXTURE_CUBE_MAP, *cubeDepthMaps[i]);

			// for each side of cubemap
			for (unsigned int j = 0; j < 6; j++) {
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, 0, GL_DEPTH_COMPONENT, shadowWidth, shadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			}
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			//float borderColour[] = { 1.0f, 1.0f, 1.0f, 1.0f };
			//glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColour);
		}
	}

	void RenderManager::BindShadowMapTextureToFramebuffer(int mapIndex, DepthMapType type)
	{
		if (mapIndex == -1) {
			Bind2DMap(depthMap);
		}
		else if (mapIndex < 8) {
			if (type == MAP_2D) {
				Bind2DMap(flatDepthMaps[mapIndex]);
			}
			else if (type == MAP_CUBE) {
				BindCubeMap(cubeDepthMaps[mapIndex]);
			}
		}
		else {
			return;
		}
	}

	void RenderManager::Bind2DMap(unsigned int* map)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, *flatDepthMapFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *map, 0);

		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void RenderManager::BindCubeMap(unsigned int* map)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, *cubeDepthMapFBO);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, *map, 0);

		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	unsigned int* RenderManager::GetDepthMap(int index, DepthMapType type)
	{
		if (index >= 8) {
			return nullptr;
		}
		else if (index == -1) {
			return depthMap;
		}
		else {
			if (type == MAP_2D) {
				return flatDepthMaps[index];
			}
			else if (type == MAP_CUBE) {
				return cubeDepthMaps[index];
			}
		}
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
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, *cubeDepthMaps[0], 0);

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
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
		
		// Attach texture to fbo
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *screenTexture, 0);
		
		// Generate render buffer object for depth and stencil testing that won't be sampled
		unsigned int rbo;
		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screenWidth, screenHeight);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}