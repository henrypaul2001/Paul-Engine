#include "RenderPipeline.h"
#include "SystemRender.h"'
#include "SystemShadowMapping.h"
#include "SystemUIRender.h"
#include "ComponentLight.h"
#include "LightManager.h"
#include "ResourceManager.h"
#include "RenderManager.h"
namespace Engine {
	RenderPipeline::RenderPipeline()
	{
		textureLookups = &ResourceManager::GetInstance()->GetTextureSlotLookupMap();
	}

	RenderPipeline::~RenderPipeline()
	{

	}

	void RenderPipeline::Run(std::vector<System*> renderSystems, std::vector<Entity*> entities)
	{
		this->entities = entities;

		shadowmapSystem = nullptr;
		renderSystem = nullptr;
		uiRenderSystem = nullptr;
		particleRenderSystem = nullptr;
		renderInstance = RenderManager::GetInstance();

		shadowWidth = renderInstance->ShadowWidth();
		shadowHeight = renderInstance->ShadowHeight();

		screenWidth = renderInstance->ScreenWidth();
		screenHeight = renderInstance->ScreenHeight();

		for (System* s : renderSystems) {
			if (s->Name() == SYSTEM_RENDER) {
				renderSystem = dynamic_cast<SystemRender*>(s);
			}
			else if (s->Name() == SYSTEM_SHADOWMAP) {
				shadowmapSystem = dynamic_cast<SystemShadowMapping*>(s);
			}
			else if (s->Name() == SYSTEM_UI_RENDER) {
				uiRenderSystem = dynamic_cast<SystemUIRender*>(s);
			}
			else if (s->Name() == SYSTEM_PARTICLE_RENDER) {
				particleRenderSystem = dynamic_cast<SystemParticleRenderer*>(s);
			}
		}
	}

	void RenderPipeline::DirLightShadowStep()
	{
		if (LightManager::GetInstance()->GetDirectionalLightEntity() != nullptr) {
			// Directional light
			unsigned int shadowWidth = renderInstance->ShadowWidth();
			unsigned int shadowHeight = renderInstance->ShadowHeight();

			ComponentLight* dirLight = dynamic_cast<ComponentLight*>(LightManager::GetInstance()->GetDirectionalLightEntity()->GetComponent(COMPONENT_LIGHT));

			if (dirLight->CastShadows) {
				glm::vec3 lightPos = -dirLight->Direction * dirLight->DirectionalLightDistance; // negative of the directional light's direction
				float orthoSize = dirLight->ShadowProjectionSize;
				float near = dirLight->Near;
				float far = dirLight->Far;
				glm::mat4 lightProjection = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, near, far);
				glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				glm::mat4 lightSpaceMatrix = lightProjection * lightView;

				depthShader->Use();
				depthShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);

				glViewport(0, 0, shadowWidth, shadowHeight);
				glBindFramebuffer(GL_FRAMEBUFFER, *depthMapFBO);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, renderInstance->GetDirectionalShadowMap(), 0);
				glClear(GL_DEPTH_BUFFER_BIT);

				shadowmapSystem->SetDepthMapType(MAP_2D);
				for (Entity* e : entities) {
					shadowmapSystem->OnAction(e);
				}
				shadowmapSystem->AfterAction();
			}
		}
	}

	void RenderPipeline::ActiveLightsShadowStep()
	{
		// Spot and point lights
		std::vector<glm::mat4> shadowTransforms;

		const FlatTextureAtlas* flatShadowAtlas = renderInstance->GetFlatShadowmapTextureAtlas();
		unsigned int numFlatShadowColumns = flatShadowAtlas->GetNumColumns();

		unsigned int pointShadowArray = renderInstance->GetPointLightCubemapShadowArray();

		// Clear spotlight texture atlas
		glBindFramebuffer(GL_FRAMEBUFFER, *depthMapFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, flatShadowAtlas->GetTextureID(), 0);
		glClear(GL_DEPTH_BUFFER_BIT);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, *cubeDepthMapFBO);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, pointShadowArray, 0);
		glClear(GL_DEPTH_BUFFER_BIT);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		float aspect = (float)shadowWidth / (float)shadowHeight;
		std::vector<Entity*> lightEntities = LightManager::GetInstance()->GetLightEntities();
		for (int i = 0; i < lightEntities.size() && i < 8; i++) {
			ComponentLight* lightComponent = dynamic_cast<ComponentLight*>(lightEntities[i]->GetComponent(COMPONENT_LIGHT));
			ComponentTransform* transformComponent = dynamic_cast<ComponentTransform*>(lightEntities[i]->GetComponent(COMPONENT_TRANSFORM));

			if (lightComponent->CastShadows) {
				if (lightComponent->GetLightType() == SPOT) {
					glBindTexture(GL_TEXTURE_2D, 0);
					glBindFramebuffer(GL_FRAMEBUFFER, *depthMapFBO);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, flatShadowAtlas->GetTextureID(), 0);
					glDrawBuffer(GL_NONE);
					glReadBuffer(GL_NONE);

					glm::vec3 lightPos = transformComponent->GetWorldPosition();
					glm::mat4 lightProjection = glm::perspective(glm::radians(90.0f), aspect, lightComponent->Near, lightComponent->Far);
					glm::mat4 lightView = glm::lookAt(lightPos, lightPos + lightComponent->WorldDirection, glm::vec3(0.0f, 1.0f, 0.0f));
					glm::mat4 lightSpaceMatrix = lightProjection * lightView;

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

					shadowmapSystem->SetDepthMapType(MAP_2D);
					for (Entity* e : entities) {
						shadowmapSystem->OnAction(e);
					}
					shadowmapSystem->AfterAction();
					glBindFramebuffer(GL_FRAMEBUFFER, 0);
				}
				else if (lightComponent->GetLightType() == POINT) {
					glBindFramebuffer(GL_FRAMEBUFFER, *cubeDepthMapFBO);
					glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, pointShadowArray, 0);
					glDrawBuffer(GL_NONE);
					glReadBuffer(GL_NONE);

					glm::mat4 lightProjection = glm::perspective(glm::radians(90.0f), aspect, lightComponent->Near, lightComponent->Far);
					glm::vec3 lightPos = transformComponent->GetWorldPosition();

					shadowTransforms.clear();
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

					shadowmapSystem->SetDepthMapType(MAP_CUBE);
					for (Entity* e : entities) {
						shadowmapSystem->OnAction(e);
					}
					shadowmapSystem->AfterAction();
					glBindFramebuffer(GL_FRAMEBUFFER, 0);
				}
			}
		}
	}

	void RenderPipeline::RunShadowMapSteps()
	{
		RenderOptions renderOptions = renderInstance->GetRenderParams()->GetRenderOptions();
		if ((renderOptions & RENDER_SHADOWS) != 0) {
			depthShader = ResourceManager::GetInstance()->ShadowMapShader();
			cubeDepthShader = ResourceManager::GetInstance()->CubeShadowMapShader();
			depthMapFBO = renderInstance->GetFlatDepthFBO();
			cubeDepthMapFBO = renderInstance->GetCubeDepthFBO();

			DirLightShadowStep();

			ActiveLightsShadowStep();
		}
	}

	void RenderPipeline::BloomStep()
	{
		RenderOptions renderOptions = renderInstance->GetRenderParams()->GetRenderOptions();
		if ((renderOptions & RENDER_BLOOM) != 0 && (renderOptions & RENDER_ADVANCED_BLOOM) == 0) {
			bool horizontal = true;
			bool first_iteration = true;
			int bloomPasses = renderInstance->GetRenderParams()->GetBloomPasses();
			Shader* blurShader = ResourceManager::GetInstance()->BloomBlurShader();
			blurShader->Use();

			glDisable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);
			glActiveTexture(GL_TEXTURE0);
			for (unsigned int i = 0; i < bloomPasses; i++) {
				glBindFramebuffer(GL_FRAMEBUFFER, *renderInstance->GetBloomPingPongFBO(horizontal));
				blurShader->setInt("horizontal", horizontal);
				glBindTexture(GL_TEXTURE_2D, first_iteration ? *renderInstance->GetBloomBrightnessTexture() : *renderInstance->GetBloomPingPongColourBuffer(!horizontal));

				ResourceManager::GetInstance()->DefaultPlane().DrawWithNoMaterial();

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
		RenderOptions renderOptions = renderInstance->GetRenderParams()->GetRenderOptions();
		if ((renderOptions & RENDER_UI) != 0) {
			if (uiRenderSystem != nullptr) {
				glViewport(0, 0, screenWidth, screenHeight);
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				//glDrawBuffer(GL_COLOR_ATTACHMENT0);

				for (Entity* e : entities) {
					uiRenderSystem->OnAction(e);
				}
				uiRenderSystem->AfterAction();
			}
		}
	}

	void RenderPipeline::ForwardParticleRenderStep()
	{
		if (particleRenderSystem) {
			// Render particles
			for (Entity* e : entities) {
				particleRenderSystem->OnAction(e);
			}
			particleRenderSystem->AfterAction();
		}
	}

	void RenderPipeline::AdvBloomCombineStep(const bool renderDirtMask, const float bloomStrength, const float lensDirtStrength)
	{
		glViewport(0, 0, screenWidth, screenHeight);
		glBindFramebuffer(GL_FRAMEBUFFER, *renderInstance->GetTexturedFBO());
		glDrawBuffer(GL_COLOR_ATTACHMENT0);

		Shader* bloomCombineShader = ResourceManager::GetInstance()->AdvBloomCombineShader();
		bloomCombineShader->Use();
		bloomCombineShader->setFloat("bloomStrength", bloomStrength);
		bloomCombineShader->setFloat("dirtMaskStrength", lensDirtStrength);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, *renderInstance->GetScreenTexture());
		
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

		ResourceManager::GetInstance()->DefaultPlane().DrawWithNoMaterial();
	}

	void RenderPipeline::AdvancedBloomStep()
	{
		RenderParams* renderParams = renderInstance->GetRenderParams();
		RenderOptions renderOptions = renderParams->GetRenderOptions();
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
		Shader* downsampleShader = ResourceManager::GetInstance()->AdvBloomDownsampleShader();
		downsampleShader->Use();
		downsampleShader->setVec2("srcResolution", glm::vec2((float)screenWidth, (float)screenHeight));
		downsampleShader->setFloat("threshold", threshold);
		downsampleShader->setFloat("softThreshold", softThreshold);
		downsampleShader->setFloat("gamma", renderInstance->GetRenderParams()->GetGamma());
		downsampleShader->setBool("firstIteration", true);

		glActiveTexture(GL_TEXTURE0);
		glDisable(GL_BLEND);
		unsigned int previousTexture = *renderInstance->GetScreenTexture();
		Mesh defaultPlane = ResourceManager::GetInstance()->DefaultPlane();

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
		Shader* upsampleShader = ResourceManager::GetInstance()->AdvBloomUpsampleShader();
		upsampleShader->Use();
		upsampleShader->setFloat("filterRadius", filterRadius);
		upsampleShader->setFloat("aspectRatio", (float)screenWidth / (float)screenHeight);

		Mesh defaultPlane = ResourceManager::GetInstance()->DefaultPlane();

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