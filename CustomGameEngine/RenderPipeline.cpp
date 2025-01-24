#include "RenderPipeline.h"
#include "RenderManager.h"

namespace Engine {
	void RenderPipeline::Run(EntityManagerNew* ecs, LightManager* lightManager, CollisionManager* collisionManager)
	{
		SCOPE_TIMER("RenderPipeline::Run");
		this->ecs = ecs;
		this->lightManager = lightManager;
		this->collisionManager = collisionManager;

		renderInstance = RenderManager::GetInstance();
		shadowWidth = renderInstance->ShadowWidth();
		shadowHeight = renderInstance->ShadowHeight();

		screenWidth = renderInstance->ScreenWidth();
		screenHeight = renderInstance->ScreenHeight();

		renderSystem.ecs = ecs;
		renderSystem.lightManager = lightManager;
		colliderDebugRenderSystem.collisionManager = collisionManager;
	}

	void RenderPipeline::DirLightShadowStep()
	{
		SCOPE_TIMER("RenderPipeline::DirLightShadowStep");
		const std::vector<unsigned int>& dirLightEntityIDs = lightManager->GetDirectionalLightEntities();
		if (dirLightEntityIDs.size() > 0) {
			// Directional light
			ComponentLight* dirLight = ecs->GetComponent<ComponentLight>(dirLightEntityIDs[0]);

			if (dirLight->CastShadows) {
				const glm::vec3 lightPos = -dirLight->Direction * dirLight->DirectionalLightDistance; // negative of the directional light's direction
				const float orthoSize = dirLight->ShadowProjectionSize;
				const float near = dirLight->Near;
				const float far = dirLight->Far;
				const glm::mat4 lightProjection = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, near, far);
				const glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				const glm::mat4 lightSpaceMatrix = lightProjection * lightView;

				depthShader->Use();
				depthShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);

				glViewport(0, 0, shadowWidth, shadowHeight);
				glBindFramebuffer(GL_FRAMEBUFFER, *depthMapFBO);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, renderInstance->GetDirectionalShadowMap(), 0);
				glClear(GL_DEPTH_BUFFER_BIT);

				shadowmapSystem.SetDepthMapType(MAP_2D);
				//for (Entity* e : entities) {
					//shadowmapSystem->OnAction(e);
				//}
				shadowmapSystem.AfterAction();
			}
		}
	}

	void RenderPipeline::ActiveLightsShadowStep()
	{
		SCOPE_TIMER("RenderPipeline::ActiveLightsShadowStep");
		// Spot and point lights

		const FlatTextureAtlas* flatShadowAtlas = renderInstance->GetFlatShadowmapTextureAtlas();
		const unsigned int numFlatShadowColumns = flatShadowAtlas->GetNumColumns();

		const unsigned int pointShadowArray = renderInstance->GetPointLightCubemapShadowArray();

		// Clear spotlight texture atlas
		glBindFramebuffer(GL_FRAMEBUFFER, *depthMapFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, flatShadowAtlas->GetTextureID(), 0);
		glClear(GL_DEPTH_BUFFER_BIT);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, *cubeDepthMapFBO);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, pointShadowArray, 0);
		glClear(GL_DEPTH_BUFFER_BIT);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		const float aspect = (float)shadowWidth / (float)shadowHeight;
		const std::map<float, unsigned int>& lightEntities = lightManager->GetLightEntities();
		std::map<float, unsigned int>::const_iterator lightEntitiesIt = lightEntities.begin();
		for (int i = 0; i < lightEntities.size() && i < 8; i++) {
			const ComponentTransform* transformComponent = ecs->GetComponent<ComponentTransform>(lightEntitiesIt->second);
			const ComponentLight* lightComponent = ecs->GetComponent<ComponentLight>(lightEntitiesIt->second);

			if (lightComponent->CastShadows) {
				if (lightComponent->GetLightType() == SPOT) {
					glBindTexture(GL_TEXTURE_2D, 0);
					glBindFramebuffer(GL_FRAMEBUFFER, *depthMapFBO);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, flatShadowAtlas->GetTextureID(), 0);
					glDrawBuffer(GL_NONE);
					glReadBuffer(GL_NONE);

					const glm::vec3& lightPos = transformComponent->GetWorldPosition();
					const glm::mat4 lightProjection = glm::perspective(glm::radians(90.0f), aspect, lightComponent->Near, lightComponent->Far);
					const glm::mat4 lightView = glm::lookAt(lightPos, lightPos + lightComponent->WorldDirection, glm::vec3(0.0f, 1.0f, 0.0f));
					const glm::mat4 lightSpaceMatrix = lightProjection * lightView;

					depthShader->Use();
					depthShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);

					unsigned int slotRow, slotColumn;
					if (i < numFlatShadowColumns) {
						slotRow = 0;
						slotColumn = i;
					}
					else {
						slotRow = i / numFlatShadowColumns;
						slotColumn = i % numFlatShadowColumns;
					}
					glm::uvec2 startXY = flatShadowAtlas->GetSlotStartXY(slotRow, slotColumn);

					glViewport(startXY.x, startXY.y, shadowWidth, shadowHeight);

					shadowmapSystem.SetDepthMapType(MAP_2D);
					//for (Entity* e : entities) {
						//shadowmapSystem.OnAction(e);
					//}
					shadowmapSystem.AfterAction();
					glBindFramebuffer(GL_FRAMEBUFFER, 0);
				}
				else if (lightComponent->GetLightType() == POINT) {
					glBindFramebuffer(GL_FRAMEBUFFER, *cubeDepthMapFBO);
					glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, pointShadowArray, 0);
					glDrawBuffer(GL_NONE);
					glReadBuffer(GL_NONE);

					const glm::mat4 lightProjection = glm::perspective(glm::radians(90.0f), aspect, lightComponent->Near, lightComponent->Far);
					const glm::vec3& lightPos = transformComponent->GetWorldPosition();

					std::vector<glm::mat4> shadowTransforms;
					shadowTransforms.push_back(lightProjection * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
					shadowTransforms.push_back(lightProjection * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
					shadowTransforms.push_back(lightProjection * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
					shadowTransforms.push_back(lightProjection * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
					shadowTransforms.push_back(lightProjection * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
					shadowTransforms.push_back(lightProjection * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

					cubeDepthShader->Use();
					for (unsigned int j = 0; j < 6; ++j) {
						cubeDepthShader->setMat4("shadowMatrices[" + std::to_string(j) + "]", shadowTransforms[j]);
						cubeDepthShader->setFloat("far_plane", lightComponent->Far);
						cubeDepthShader->setVec3("lightPos", lightPos);
					}
					cubeDepthShader->setInt("cubemapIndex", i);

					glViewport(0, 0, shadowWidth, shadowHeight);

					shadowmapSystem.SetDepthMapType(MAP_CUBE);
					//for (Entity* e : entities) {
						//shadowmapSystem.OnAction(e);
					//}
					shadowmapSystem.AfterAction();
					glBindFramebuffer(GL_FRAMEBUFFER, 0);
				}
			}
			lightEntitiesIt++;
		}
	}

	void RenderPipeline::RunShadowMapSteps()
	{
		SCOPE_TIMER("RenderPipeline::RunShadowMapSteps");
		RenderOptions renderOptions = renderInstance->GetRenderParams()->GetRenderOptions();
		if ((renderOptions & RENDER_SHADOWS) != 0) {
			depthShader = resources->ShadowMapShader();
			cubeDepthShader = resources->CubeShadowMapShader();
			depthMapFBO = renderInstance->GetFlatDepthFBO();
			cubeDepthMapFBO = renderInstance->GetCubeDepthFBO();

			DirLightShadowStep();

			ActiveLightsShadowStep();
		}
	}

	void RenderPipeline::BloomStep(const unsigned int activeBloomTexture)
	{
		SCOPE_TIMER("RenderPipeline::BloomStep");
		const RenderOptions& renderOptions = renderInstance->GetRenderParams()->GetRenderOptions();
		if ((renderOptions & RENDER_BLOOM) != 0 && (renderOptions & RENDER_ADVANCED_BLOOM) == 0) {
			bool horizontal = true;
			bool first_iteration = true;
			const int bloomPasses = renderInstance->GetRenderParams()->GetBloomPasses();
			Shader* blurShader = resources->BloomBlurShader();
			blurShader->Use();

			glDisable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);
			glActiveTexture(GL_TEXTURE0);
			for (unsigned int i = 0; i < bloomPasses; i++) {
				glBindFramebuffer(GL_FRAMEBUFFER, *renderInstance->GetBloomPingPongFBO(horizontal));
				blurShader->setInt("horizontal", horizontal);
				glBindTexture(GL_TEXTURE_2D, first_iteration ? activeBloomTexture : *renderInstance->GetBloomPingPongColourBuffer(!horizontal));

				resources->DefaultPlane().DrawWithNoMaterial();

				horizontal = !horizontal;
				if (first_iteration) {
					first_iteration = false;
				}
			}
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);

			finalBloomTexture = !horizontal;

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	}

	void RenderPipeline::UIRenderStep()
	{
		SCOPE_TIMER("RenderPipeline::UIRenderStep");
		RenderOptions renderOptions = renderInstance->GetRenderParams()->GetRenderOptions();
		if ((renderOptions & RENDER_UI) != 0) {
			glViewport(0, 0, screenWidth, screenHeight);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			//glDrawBuffer(GL_COLOR_ATTACHMENT0);

			//for (Entity* e : entities) {
				//uiRenderSystem.OnAction(e);
			//}
			uiRenderSystem.AfterAction();
		}
	}

	void RenderPipeline::ForwardParticleRenderStep()
	{
		SCOPE_TIMER("RenderPipeline::ForwardParticleRenderStep");
		// Render particles
		//for (Entity* e : entities) {
			//particleRenderSystem.OnAction(e);
		//}
		particleRenderSystem.AfterAction();
	}

	void RenderPipeline::AdvBloomCombineStep(const bool renderDirtMask, const float bloomStrength, const float lensDirtStrength)
	{
		SCOPE_TIMER("RenderPipeline::AdvBloomCombineStep");
		glViewport(0, 0, screenWidth, screenHeight);
		glBindFramebuffer(GL_FRAMEBUFFER, *renderInstance->GetTexturedFBO());
		glDrawBuffer(GL_COLOR_ATTACHMENT0);

		Shader* bloomCombineShader = resources->AdvBloomCombineShader();
		bloomCombineShader->Use();
		bloomCombineShader->setFloat("bloomStrength", bloomStrength);
		bloomCombineShader->setFloat("dirtMaskStrength", lensDirtStrength);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, advBloomSourceTexture);
		
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, renderInstance->GetAdvBloomMipChain()[0].texture);

		const Texture* lensDirt = renderInstance->GetAdvBloomLensDirtTexture();
		if (renderDirtMask && lensDirt != nullptr) {
			bloomCombineShader->setBool("useDirtMask", true);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, lensDirt->id);
		}
		else {
			bloomCombineShader->setBool("useDirtMask", false);
		}

		resources->DefaultPlane().DrawWithNoMaterial();
	}

	void RenderPipeline::AdvancedBloomStep(const unsigned int activeScreenTexture)
	{
		SCOPE_TIMER("RenderPipeline::AdvancedBloomStep");
		const RenderParams* renderParams = renderInstance->GetRenderParams();
		const RenderOptions& renderOptions = renderParams->GetRenderOptions();
		advBloomSourceTexture = activeScreenTexture;
		if ((renderOptions & RENDER_ADVANCED_BLOOM) != 0) {
			unsigned int bloomFBO = *renderInstance->GetAdvBloomFBO();
			glBindFramebuffer(GL_FRAMEBUFFER, bloomFBO);
			glClear(GL_COLOR_BUFFER_BIT);
			
			std::vector<AdvBloomMip> mipChain = renderInstance->GetAdvBloomMipChain();

			// Downsample
			AdvBloomDownsampleStep(mipChain, renderParams->GetAdvBloomThreshold(), renderParams->GetAdvBloomSoftThreshold());

			// Upsample
			AdvBloomUpsampleStep(mipChain, renderParams->GetAdvBloomFilterRadius());

			// Combine
			AdvBloomCombineStep((renderOptions & RENDER_ADVANCED_BLOOM_LENS_DIRT) != 0, renderParams->GetAdvBloomStrength(), renderParams->GetAdvBloomLensDirtMaskStrength());
		}
	}

	void RenderPipeline::AdvBloomDownsampleStep(const std::vector<AdvBloomMip>& mipChain, const float threshold, const float softThreshold)
	{
		SCOPE_TIMER("RenderPipeline::AdvBloomDownsampleStep");
		Shader* downsampleShader = resources->AdvBloomDownsampleShader();
		downsampleShader->Use();
		downsampleShader->setVec2("srcResolution", glm::vec2((float)screenWidth, (float)screenHeight));
		downsampleShader->setFloat("threshold", threshold);
		downsampleShader->setFloat("softThreshold", softThreshold);
		downsampleShader->setFloat("gamma", renderInstance->GetRenderParams()->GetGamma());
		downsampleShader->setBool("firstIteration", true);

		glActiveTexture(GL_TEXTURE0);
		glDisable(GL_BLEND);
		unsigned int previousTexture = advBloomSourceTexture;
		Mesh defaultPlane = resources->DefaultPlane();

		// Progressively downsample through mip chain
		for (int i = 0; i < mipChain.size(); i++) {
			const AdvBloomMip& mip = mipChain[i];

			if (i > 0) {
				downsampleShader->setBool("firstIteration", false);
			}

			glViewport(0, 0, mip.size.x, mip.size.y);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mip.texture, 0);

			glBindTexture(GL_TEXTURE_2D, previousTexture);

			defaultPlane.DrawWithNoMaterial();

			// Set current mip as src resolution for next iteration
			downsampleShader->setVec2("srcResolution", mip.size);
			previousTexture = mip.texture;
		}
	}

	void RenderPipeline::AdvBloomUpsampleStep(const std::vector<AdvBloomMip>& mipChain, const float filterRadius)
	{
		SCOPE_TIMER("RenderPipeline::AdvBloomUpsampleStep");
		Shader* upsampleShader = resources->AdvBloomUpsampleShader();
		upsampleShader->Use();
		upsampleShader->setFloat("filterRadius", filterRadius);
		upsampleShader->setFloat("aspectRatio", (float)screenWidth / (float)screenHeight);

		Mesh defaultPlane = resources->DefaultPlane();

		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);

		for (int i = mipChain.size() - 1; i > 0; i--) {
			const AdvBloomMip& mip = mipChain[i];
			const AdvBloomMip& nextMip = mipChain[i - 1];

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, mip.texture);

			// Set render target
			glViewport(0, 0, nextMip.size.x, nextMip.size.y);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, nextMip.texture, 0);

			defaultPlane.DrawWithNoMaterial();
		}

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_BLEND);
	}
}