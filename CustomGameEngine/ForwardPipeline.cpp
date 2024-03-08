#include "ForwardPipeline.h"
#include "ResourceManager.h"
#include "ComponentLight.h"
#include "LightManager.h"

#include "SystemRender.h"'
#include "SystemShadowMapping.h"
namespace Engine {
	ForwardPipeline::ForwardPipeline()
	{

	}

	ForwardPipeline::~ForwardPipeline()
	{

	}

	void ForwardPipeline::Run(std::vector<System*> renderSystems, std::vector<Entity*> entities)
	{
		RenderPipeline::Run(renderSystems, entities);

		// shadow map steps
		if (shadowmapSystem != nullptr) {
			depthShader = ResourceManager::GetInstance()->ShadowMapShader();
			cubeDepthShader = ResourceManager::GetInstance()->CubeShadowMapShader();
			depthMapFBO = renderInstance->GetFlatDepthFBO();
			cubeDepthMapFBO = renderInstance->GetCubeDepthFBO();

			DirLightShadowStep();

			ActiveLightsShadowStep();
		}

		// render scene to textured framebuffer
		if (renderSystem != nullptr) {

			SceneRenderStep();

			ScreenTextureStep();
		}
	}

	void ForwardPipeline::DirLightShadowStep()
	{
		// Directional light
		renderInstance->BindShadowMapTextureToFramebuffer(-1, MAP_2D); // bind the dir light shadowmap to framebuffer
		unsigned int shadowWidth = renderInstance->ShadowWidth(); // in future, these will be stored in the light component
		unsigned int shadowHeight = renderInstance->ShadowHeight(); // <--/

		ComponentLight* dirLight = dynamic_cast<ComponentLight*>(LightManager::GetInstance()->GetDirectionalLightEntity()->GetComponent(COMPONENT_LIGHT));
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

	void ForwardPipeline::ActiveLightsShadowStep()
	{
		// Spot and point lights
		std::vector<glm::mat4> shadowTransforms;
		float aspect = (float)shadowWidth / (float)shadowHeight;
		std::vector<Entity*> lightEntities = LightManager::GetInstance()->GetLightEntities();
		for (int i = 0; i < lightEntities.size() && i < 8; i++) {
			ComponentLight* lightComponent = dynamic_cast<ComponentLight*>(lightEntities[i]->GetComponent(COMPONENT_LIGHT));
			ComponentTransform* transformComponent = dynamic_cast<ComponentTransform*>(lightEntities[i]->GetComponent(COMPONENT_TRANSFORM));

			if (lightComponent->GetLightType() == SPOT) {
				renderInstance->BindShadowMapTextureToFramebuffer(i, MAP_2D);

				glm::vec3 lightPos = transformComponent->GetWorldPosition();
				glm::mat4 lightProjection = glm::perspective(glm::radians(90.0f), aspect, lightComponent->Near, lightComponent->Far);
				glm::mat4 lightView = glm::lookAt(lightPos, lightPos + lightComponent->Direction, glm::vec3(0.0f, 1.0f, 0.0f));
				glm::mat4 lightSpaceMatrix = lightProjection * lightView;

				depthShader->Use();
				depthShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);

				glViewport(0, 0, shadowWidth, shadowHeight);
				glBindFramebuffer(GL_FRAMEBUFFER, *depthMapFBO);
				glClear(GL_DEPTH_BUFFER_BIT);

				//glEnable(GL_CULL_FACE);
				//glCullFace(GL_FRONT);
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

	void ForwardPipeline::SceneRenderStep()
	{
		glViewport(0, 0, screenWidth, screenHeight);

		// Render to textured framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, *renderInstance->GetTexturedFBO());

		//glCullFace(GL_BACK);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		for (Entity* e : entities) {
			renderSystem->OnAction(e);
		}
		renderSystem->AfterAction();
	}

	void ForwardPipeline::ScreenTextureStep()
	{
		// render final scene texture on screen quad
		glViewport(0, 0, screenWidth, screenHeight);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT);

		Shader* screenQuadShader = ResourceManager::GetInstance()->ScreenQuadShader();
		screenQuadShader->Use();

		screenQuadShader->setUInt("postProcess", renderSystem->GetPostProcess());

		if (renderSystem->GetPostProcess() == CUSTOM_KERNEL) {
			screenQuadShader->setFloat("customKernel[0]", renderSystem->PostProcessKernel[0]);
			screenQuadShader->setFloat("customKernel[1]", renderSystem->PostProcessKernel[1]);
			screenQuadShader->setFloat("customKernel[2]", renderSystem->PostProcessKernel[2]);
			screenQuadShader->setFloat("customKernel[3]", renderSystem->PostProcessKernel[3]);
			screenQuadShader->setFloat("customKernel[4]", renderSystem->PostProcessKernel[4]);
			screenQuadShader->setFloat("customKernel[5]", renderSystem->PostProcessKernel[5]);
			screenQuadShader->setFloat("customKernel[6]", renderSystem->PostProcessKernel[6]);
			screenQuadShader->setFloat("customKernel[7]", renderSystem->PostProcessKernel[7]);
			screenQuadShader->setFloat("customKernel[8]", renderSystem->PostProcessKernel[8]);
		}

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, *renderInstance->GetScreenTexture());
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		ResourceManager::GetInstance()->DefaultPlane()->Draw(*screenQuadShader);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
	}
}