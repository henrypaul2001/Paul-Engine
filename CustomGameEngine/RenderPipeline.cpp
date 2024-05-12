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
		}
	}

	void RenderPipeline::DirLightShadowStep()
	{
		if (LightManager::GetInstance()->GetDirectionalLightEntity() != nullptr) {
			// Directional light
			renderInstance->BindShadowMapTextureToFramebuffer(-1, MAP_2D); // bind the dir light shadowmap to framebuffer
			unsigned int shadowWidth = renderInstance->ShadowWidth(); // in future, these will be stored in the light component
			unsigned int shadowHeight = renderInstance->ShadowHeight(); // <--/

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
		float aspect = (float)shadowWidth / (float)shadowHeight;
		std::vector<Entity*> lightEntities = LightManager::GetInstance()->GetLightEntities();
		for (int i = 0; i < lightEntities.size() && i < 8; i++) {
			ComponentLight* lightComponent = dynamic_cast<ComponentLight*>(lightEntities[i]->GetComponent(COMPONENT_LIGHT));
			ComponentTransform* transformComponent = dynamic_cast<ComponentTransform*>(lightEntities[i]->GetComponent(COMPONENT_TRANSFORM));
			if (lightComponent->CastShadows) {
				if (lightComponent->GetLightType() == SPOT) {
					renderInstance->BindShadowMapTextureToFramebuffer(i, MAP_2D);

					glm::vec3 lightPos = transformComponent->GetWorldPosition();
					glm::mat4 lightProjection = glm::perspective(glm::radians(90.0f), aspect, lightComponent->Near, lightComponent->Far);
					glm::mat4 lightView = glm::lookAt(lightPos, lightPos + lightComponent->WorldDirection, glm::vec3(0.0f, 1.0f, 0.0f));
					glm::mat4 lightSpaceMatrix = lightProjection * lightView;

					depthShader->Use();
					depthShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);

					glViewport(0, 0, shadowWidth, shadowHeight);
					glBindFramebuffer(GL_FRAMEBUFFER, *depthMapFBO);
					glClear(GL_DEPTH_BUFFER_BIT);

					shadowmapSystem->SetDepthMapType(MAP_2D);
					for (Entity* e : entities) {
						shadowmapSystem->OnAction(e);
					}
					shadowmapSystem->AfterAction();
				}
				else if (lightComponent->GetLightType() == POINT) {
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
					for (unsigned int i = 0; i < 6; ++i) {
						cubeDepthShader->setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
						cubeDepthShader->setFloat("far_plane", lightComponent->Far);
						cubeDepthShader->setVec3("lightPos", lightPos);
					}

					renderInstance->BindShadowMapTextureToFramebuffer(i, MAP_CUBE);
					glViewport(0, 0, shadowWidth, shadowHeight);
					glBindFramebuffer(GL_FRAMEBUFFER, *cubeDepthMapFBO);
					glClear(GL_DEPTH_BUFFER_BIT);

					shadowmapSystem->SetDepthMapType(MAP_CUBE);
					for (Entity* e : entities) {
						shadowmapSystem->OnAction(e);
					}
					shadowmapSystem->AfterAction();
				}
			}
		}
	}

	void RenderPipeline::RunShadowMapSteps()
	{
		RenderOptions renderOptions = renderInstance->GetRenderOptions();
		if ((renderOptions & RENDER_SHADOWS) != 0) {
			depthShader = ResourceManager::GetInstance()->ShadowMapShader();
			cubeDepthShader = ResourceManager::GetInstance()->CubeShadowMapShader();
			depthMapFBO = renderInstance->GetFlatDepthFBO();
			cubeDepthMapFBO = renderInstance->GetCubeDepthFBO();

			DirLightShadowStep();

			ActiveLightsShadowStep();
		}
	}

	void RenderPipeline::RunBloomStep()
	{
		RenderOptions renderOptions = renderInstance->GetRenderOptions();
		if ((renderOptions & RENDER_BLOOM) != 0) {
			bool horizontal = true;
			bool first_iteration = true;
			int bloomPasses = 20;
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
		RenderOptions renderOptions = renderInstance->GetRenderOptions();
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
}