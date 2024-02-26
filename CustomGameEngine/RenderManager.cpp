#include "RenderManager.h"
#include <glad/glad.h>
#include <cstddef>
namespace Engine {
	RenderManager* RenderManager::instance = nullptr;
	RenderManager::RenderManager()
	{
		depthMapFBO = new unsigned int;
		depthMap = new unsigned int;
	}

	RenderManager* RenderManager::GetInstance()
	{
		if (instance == nullptr) {
			instance = new RenderManager();
		}
		return instance;
	}

	RenderManager::~RenderManager()
	{
		delete depthMap;
		delete depthMapFBO;

		delete instance;
	}

	void RenderManager::SetupShadowMapFBO(unsigned int shadowWidth, unsigned int shadowHeight)
	{
		this->shadowHeight = shadowHeight;
		this->shadowWidth = shadowWidth;

		glGenFramebuffers(1, depthMapFBO);

		glGenTextures(1, depthMap);
		glBindTexture(GL_TEXTURE_2D, *depthMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowWidth, shadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColour[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColour);

		glBindFramebuffer(GL_FRAMEBUFFER, *depthMapFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *depthMap, 0);

		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}