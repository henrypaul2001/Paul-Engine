#include "DeferredPipeline.h"
#include "RenderManager.h"
#include "SystemRender.h"
#include "SystemShadowMapping.h"
#include "ResourceManager.h"
#include "LightManager.h"
namespace Engine {
	DeferredPipeline::DeferredPipeline()
	{

	}

	DeferredPipeline::~DeferredPipeline()
	{

	}

	void DeferredPipeline::Run(std::vector<System*> renderSystems, std::vector<Entity*> entities)
	{
		RenderPipeline::Run(renderSystems, entities);

		// shadow map steps
		if (shadowmapSystem != nullptr) {
			RunShadowMapSteps();
		}

		if (renderSystem != nullptr) {
			// Geometry pass
			// -------------
			//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glViewport(0, 0, screenWidth, screenHeight);
			glBindFramebuffer(GL_FRAMEBUFFER, *renderInstance->GetGBuffer());
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glDisable(GL_BLEND);
			for (Entity* e : entities) {
				renderSystem->OnAction(e);
			}
			renderSystem->AfterAction();

			// SSAO Pass
			// ---------
			// SSAO Texture
			glBindFramebuffer(GL_FRAMEBUFFER, *renderInstance->GetSSAOFBO());
			glClear(GL_COLOR_BUFFER_BIT);

			Shader* ssaoShader = ResourceManager::GetInstance()->SSAOShader();
			ssaoShader->Use();
			ssaoShader->setInt("scr_width", screenWidth);
			ssaoShader->setInt("scr_height", screenHeight);

			// Send kernel + rotation
			std::vector<glm::vec3*> ssaoKernel = renderInstance->SSAOKernel();
			for (unsigned int i = 0; i < 64; i++) {
				ssaoShader->setVec3("samples[" + std::to_string(i) + "]", *ssaoKernel[i]);
			}

			glBindTexture(GL_TEXTURE_2D, *renderInstance->GPosition());
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, *renderInstance->GNormal());
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, *renderInstance->SSAONoiseTexture());
			ResourceManager::GetInstance()->DefaultPlane()->Draw(*ssaoShader);

			// Blur SSAO texture to remove noise
			glBindFramebuffer(GL_FRAMEBUFFER, *renderInstance->GetSSAOBlurFBO());
			glClear(GL_COLOR_BUFFER_BIT);
			Shader* ssaoBlur = ResourceManager::GetInstance()->SSABlur();
			ssaoBlur->Use();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, *renderInstance->SSAOColour());
			ResourceManager::GetInstance()->DefaultPlane()->Draw(*ssaoBlur);

			// Lighting pass
			// -------------
			glViewport(0, 0, screenWidth, screenHeight);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			Shader* lightingPass = ResourceManager::GetInstance()->DeferredLightingPass();

			lightingPass->Use();
			glActiveTexture(GL_TEXTURE18);
			glBindTexture(GL_TEXTURE_2D, *renderInstance->GPosition());
			glActiveTexture(GL_TEXTURE19);
			glBindTexture(GL_TEXTURE_2D, *renderInstance->GNormal());
			glActiveTexture(GL_TEXTURE20);
			glBindTexture(GL_TEXTURE_2D, *renderInstance->GAlbedo());
			glActiveTexture(GL_TEXTURE21);
			glBindTexture(GL_TEXTURE_2D, *renderInstance->GSpecular());
			glActiveTexture(GL_TEXTURE22);
			glBindTexture(GL_TEXTURE_2D, *renderInstance->SSAOBlurColour());

			glDisable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);
			LightManager::GetInstance()->SetShaderUniforms(lightingPass);
			ResourceManager::GetInstance()->DefaultPlane()->Draw(*lightingPass);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);

			// Skybox
			// ------

			// Retrieve depth and stencil information from gBuffer
			glBindFramebuffer(GL_READ_FRAMEBUFFER, *renderInstance->GetGBuffer());
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			glBlitFramebuffer(0, 0, screenWidth, screenHeight, 0, 0, screenWidth, screenHeight, GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			Shader* skyShader = ResourceManager::GetInstance()->SkyboxShader();
			skyShader->Use();

			Camera* activeCamera = renderSystem->GetActiveCamera();

			glBindBuffer(GL_UNIFORM_BUFFER, ResourceManager::GetInstance()->CommonUniforms());
			glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(glm::mat4(glm::mat3(activeCamera->GetViewMatrix()))));
			glBindBuffer(GL_UNIFORM_BUFFER, 0);

			glDepthFunc(GL_LEQUAL);
			glCullFace(GL_FRONT);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, activeCamera->GetSkybox()->id);
			ResourceManager::GetInstance()->DefaultCube()->Draw(*skyShader);
			glCullFace(GL_BACK);
			glDepthFunc(GL_LESS);

			glBindBuffer(GL_UNIFORM_BUFFER, ResourceManager::GetInstance()->CommonUniforms());
			glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(activeCamera->GetViewMatrix()));
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}
	}
}