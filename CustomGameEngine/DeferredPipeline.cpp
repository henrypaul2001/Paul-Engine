#include "DeferredPipeline.h"
#include "RenderManager.h"
#include "SystemRender.h"
#include "SystemShadowMapping.h"
#include "ResourceManager.h"
#include "LightManager.h"
namespace Engine {
	DeferredPipeline::DeferredPipeline(bool pbr)
	{
		this->pbr = pbr;
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
			Camera* activeCamera = renderSystem->GetActiveCamera();

			// Geometry pass
			// -------------
			//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glViewport(0, 0, screenWidth, screenHeight);
			if (pbr) {
				glBindFramebuffer(GL_FRAMEBUFFER, *renderInstance->GetGBufferPBR());
			}
			else {
				glBindFramebuffer(GL_FRAMEBUFFER, *renderInstance->GetGBuffer());
			}
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

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, *renderInstance->GPosition());
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, *renderInstance->GNormal());
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, *renderInstance->SSAONoiseTexture());
			//ResourceManager::GetInstance()->DefaultPlane().Draw(*ssaoShader);
			ResourceManager::GetInstance()->DefaultPlane().DrawWithNoMaterial();

			// Blur SSAO texture to remove noise
			glBindFramebuffer(GL_FRAMEBUFFER, *renderInstance->GetSSAOBlurFBO());
			glClear(GL_COLOR_BUFFER_BIT);
			Shader* ssaoBlur = ResourceManager::GetInstance()->SSABlur();
			ssaoBlur->Use();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, *renderInstance->SSAOColour());
			ResourceManager::GetInstance()->DefaultPlane().DrawWithNoMaterial();

			// Lighting pass
			// -------------
			glViewport(0, 0, screenWidth, screenHeight);
			glBindFramebuffer(GL_FRAMEBUFFER, *renderInstance->GetTexturedFBO());
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			const GLenum buffers[]{ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
			glDrawBuffers(2, buffers);

			glActiveTexture(GL_TEXTURE18);
			glBindTexture(GL_TEXTURE_2D, *renderInstance->GPosition());
			glActiveTexture(GL_TEXTURE19);
			glBindTexture(GL_TEXTURE_2D, *renderInstance->GNormal());
			glActiveTexture(GL_TEXTURE20);
			glBindTexture(GL_TEXTURE_2D, *renderInstance->GAlbedo());

			Shader* lightingPass = nullptr;
			if (pbr) {
				lightingPass = ResourceManager::GetInstance()->DeferredLightingPassPBR();

				glActiveTexture(GL_TEXTURE21);
				glBindTexture(GL_TEXTURE_2D, *renderInstance->GArm());

			}
			else {
				lightingPass = ResourceManager::GetInstance()->DeferredLightingPass();

				glActiveTexture(GL_TEXTURE21);
				glBindTexture(GL_TEXTURE_2D, *renderInstance->GSpecular());

			}
			glActiveTexture(GL_TEXTURE22);
			glBindTexture(GL_TEXTURE_2D, *renderInstance->SSAOBlurColour());

			glDisable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);
			lightingPass->Use();
			lightingPass->setBool("useSSAO", activeCamera->UseSSAO());
			LightManager::GetInstance()->SetShaderUniforms(lightingPass, activeCamera);
			ResourceManager::GetInstance()->DefaultPlane().DrawWithNoMaterial();
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);

			// Skybox
			// ------
			// Retrieve depth and stencil information from gBuffer
			if (pbr) {
				glBindFramebuffer(GL_READ_FRAMEBUFFER, *renderInstance->GetGBufferPBR());
			}
			else {
				glBindFramebuffer(GL_READ_FRAMEBUFFER, *renderInstance->GetGBuffer());
			}

			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, *renderInstance->GetTexturedFBO());
			glBlitFramebuffer(0, 0, screenWidth, screenHeight, 0, 0, screenWidth, screenHeight, GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
			glBindFramebuffer(GL_FRAMEBUFFER, *renderInstance->GetTexturedFBO());

			Shader* skyShader = ResourceManager::GetInstance()->SkyboxShader();
			skyShader->Use();

			glDrawBuffer(GL_COLOR_ATTACHMENT0);

			glBindBuffer(GL_UNIFORM_BUFFER, ResourceManager::GetInstance()->CommonUniforms());
			glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(glm::mat4(glm::mat3(activeCamera->GetViewMatrix()))));
			glBindBuffer(GL_UNIFORM_BUFFER, 0);

			glDepthFunc(GL_LEQUAL);
			glCullFace(GL_FRONT);

			glActiveTexture(GL_TEXTURE0);
			if (activeCamera->UseHDREnvironmentMap()) {
				glBindTexture(GL_TEXTURE_CUBE_MAP, activeCamera->GetEnvironmentMap()->cubemapID);
			}
			else {
				glBindTexture(GL_TEXTURE_CUBE_MAP, activeCamera->GetSkybox()->id);
			}
			ResourceManager::GetInstance()->DefaultCube().DrawWithNoMaterial();
			glCullFace(GL_BACK);
			glDepthFunc(GL_LESS);

			glBindBuffer(GL_UNIFORM_BUFFER, ResourceManager::GetInstance()->CommonUniforms());
			glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(activeCamera->GetViewMatrix()));
			glBindBuffer(GL_UNIFORM_BUFFER, 0);

			// Transparency using forward rendering
			// ------------------------------------
			LightManager::GetInstance()->SetShaderUniforms(ResourceManager::GetInstance()->DefaultLitShader(), activeCamera);
			LightManager::GetInstance()->SetShaderUniforms(ResourceManager::GetInstance()->DefaultLitPBR(), activeCamera);
			glEnable(GL_BLEND);
			renderSystem->DrawTransparentGeometry(true);
			glDisable(GL_BLEND);

			// Bloom
			// -----
			RunBloomStep();

			// Post Processing
			// ---------------
			
			// HDR Tonemapping
			glViewport(0, 0, screenWidth, screenHeight);
			glBindFramebuffer(GL_FRAMEBUFFER, *renderInstance->GetTexturedFBO());
			glDrawBuffer(GL_COLOR_ATTACHMENT0);

			Shader* hdrShader = ResourceManager::GetInstance()->HDRTonemappingShader();
			hdrShader->Use();
			hdrShader->setFloat("gamma", 1.2);
			hdrShader->setFloat("exposure", renderInstance->exposure);
			hdrShader->setBool("bloom", renderInstance->bloom);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, *renderInstance->GetScreenTexture());
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, *renderInstance->GetBloomPingPongColourBuffer(finalBloomTexture));
			glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			glDisable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);
			glDisable(GL_STENCIL_TEST);
			ResourceManager::GetInstance()->DefaultPlane().DrawWithNoMaterial();
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);
			glEnable(GL_STENCIL_TEST);

			// Final post process output
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
			ResourceManager::GetInstance()->DefaultPlane().DrawWithNoMaterial();
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);
		}
	}
}