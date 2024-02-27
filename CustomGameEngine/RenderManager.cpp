#include "RenderManager.h"
#include <glad/glad.h>
#include <cstddef>
namespace Engine {
	RenderManager* RenderManager::instance = nullptr;
	RenderManager::RenderManager(unsigned int shadowWidth, unsigned int shadowHeight)
	{
		depthMapFBO = new unsigned int;
		SetupShadowMapTextures(shadowWidth, shadowHeight);
		SetupShadowMapFBO();
	}

	void RenderManager::SetupShadowMapTextures(unsigned int shadowWidth, unsigned int shadowHeight)
	{
		this->shadowHeight = shadowHeight;
		this->shadowWidth = shadowWidth;

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

		for (int i = 0; i < 8; i++) {
			activeDepthMaps.push_back(new unsigned int);

			glActiveTexture(GL_TEXTURE0 + i + 1);
			glGenTextures(1, activeDepthMaps[i]);
			glBindTexture(GL_TEXTURE_2D, *activeDepthMaps[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowWidth, shadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			float borderColour[] = { 1.0f, 1.0f, 1.0f, 1.0f };
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColour);
		}
	}

	void RenderManager::BindShadowMapTextureToFramebuffer(int mapIndex)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, *depthMapFBO);
		unsigned int* map;

		if (mapIndex == -1) {
			map = depthMap;
		}
		else if (mapIndex < 8) {
			map = activeDepthMaps[mapIndex];
		}
		else {
			return;
		}

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *map, 0);

		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	RenderManager* RenderManager::GetInstance(unsigned int shadowWidth, unsigned int shadowHeight)
	{
		if (instance == nullptr) {
			instance = new RenderManager(shadowWidth, shadowHeight);
		}
		return instance;
	}

	RenderManager::~RenderManager()
	{
		delete depthMap;
		delete depthMapFBO;

		delete instance;
	}

	unsigned int* RenderManager::GetDepthMap(int index)
	{
		if (index >= 8) {
			return nullptr;
		}
		else if (index == -1) {
			return depthMap;
		}
		else {
			return activeDepthMaps[index];
		}
	}

	void RenderManager::SetupShadowMapFBO()
	{
		glGenFramebuffers(1, depthMapFBO);

		glBindFramebuffer(GL_FRAMEBUFFER, *depthMapFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *depthMap, 0);

		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}