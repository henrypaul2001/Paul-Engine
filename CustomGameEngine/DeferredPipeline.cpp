#include "DeferredPipeline.h"
#include "RenderManager.h"
#include "SystemRender.h"
#include "SystemShadowMapping.h"
#include "ResourceManager.h"
#include "LightManager.h"
namespace Engine {
	DeferredPipeline::DeferredPipeline() {}

	DeferredPipeline::~DeferredPipeline() {}

	void DeferredPipeline::Run(std::vector<System*> renderSystems, std::vector<Entity*> entities)
	{
		RenderPipeline::Run(renderSystems, entities);
		RenderOptions renderOptions = renderInstance->GetRenderParams()->GetRenderOptions();

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
			glBindFramebuffer(GL_FRAMEBUFFER, *renderInstance->GetGBuffer());
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glDisable(GL_BLEND);
			for (Entity* e : entities) {
				renderSystem->OnAction(e);
			}
			renderSystem->AfterAction();

			// SSAO Pass
			// ---------
			if ((renderOptions & RENDER_SSAO) != 0) {
				// SSAO Texture
				glBindFramebuffer(GL_FRAMEBUFFER, *renderInstance->GetSSAOFBO());
				glClear(GL_COLOR_BUFFER_BIT);
				Shader* ssaoShader = ResourceManager::GetInstance()->SSAOShader();
				ssaoShader->Use();
				ssaoShader->setInt("scr_width", screenWidth);
				ssaoShader->setInt("scr_height", screenHeight);

				RenderParams* renderParams = renderInstance->GetRenderParams();
				int samples = renderParams->GetSSAOSamples();
				float radius = renderParams->GetSSAORadius();
				float bias = renderParams->GetSSAOBias();

				ssaoShader->setInt("kernelSize", samples);
				ssaoShader->setFloat("radius", radius);
				ssaoShader->setFloat("bias", bias);

				if (samples > 64) { samples = 64; }
				else if (samples < 0) { samples = 0; }

				// Send kernel + rotation
				std::vector<glm::vec3*> ssaoKernel = renderInstance->SSAOKernel();
				for (unsigned int i = 0; i < samples; i++) {
					ssaoShader->setVec3("samples[" + std::to_string(i) + "]", *ssaoKernel[i]);
				}

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, *renderInstance->GPosition());
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, *renderInstance->GNormal());
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, *renderInstance->SSAONoiseTexture());
				ResourceManager::GetInstance()->DefaultPlane().DrawWithNoMaterial();

				// Blur SSAO texture to remove noise
				glBindFramebuffer(GL_FRAMEBUFFER, *renderInstance->GetSSAOBlurFBO());
				glClear(GL_COLOR_BUFFER_BIT);
				Shader* ssaoBlur = ResourceManager::GetInstance()->SSABlur();
				ssaoBlur->Use();
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, *renderInstance->SSAOColour());
				ResourceManager::GetInstance()->DefaultPlane().DrawWithNoMaterial();
			}

			// Lighting pass
			// -------------

			// Two lighting passes, first, non pbr will light all non pbr pixels
			// Second pass will light all pbr pixels

			glViewport(0, 0, screenWidth, screenHeight);
			glBindFramebuffer(GL_FRAMEBUFFER, *renderInstance->GetTexturedFBO());
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *renderInstance->GetAlternateScreenTexture(), 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, *renderInstance->GetAlternateBloomBrightnessTexture(), 0);

			const GLenum buffers[]{ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
			glDrawBuffers(2, buffers);

			glActiveTexture(GL_TEXTURE18);
			glBindTexture(GL_TEXTURE_2D, *renderInstance->GPosition());
			glActiveTexture(GL_TEXTURE19);
			glBindTexture(GL_TEXTURE_2D, *renderInstance->GNormal());
			glActiveTexture(GL_TEXTURE20);
			glBindTexture(GL_TEXTURE_2D, *renderInstance->GAlbedo());

			glActiveTexture(GL_TEXTURE21);
			glBindTexture(GL_TEXTURE_2D, *renderInstance->GSpecular()); // non pbr specific

			glActiveTexture(GL_TEXTURE22);
			glBindTexture(GL_TEXTURE_2D, *renderInstance->GPBRFLAG());

			glActiveTexture(GL_TEXTURE23);
			glBindTexture(GL_TEXTURE_2D, *renderInstance->SSAOBlurColour());

			// First pass
			Shader* lightingPass = ResourceManager::GetInstance()->DeferredLightingPass();
			glDisable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);
			lightingPass->Use();
			lightingPass->setBool("useSSAO", ((renderOptions & RENDER_SSAO) != 0));
			LightManager::GetInstance()->SetShaderUniforms(lightingPass, activeCamera);
			ResourceManager::GetInstance()->DefaultPlane().DrawWithNoMaterial();

			// Second pass (pbr)
			glBindFramebuffer(GL_FRAMEBUFFER, *renderInstance->GetTexturedFBO());

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *renderInstance->GetScreenTexture(), 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, *renderInstance->GetBloomBrightnessTexture(), 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glActiveTexture(GL_TEXTURE21);
			glBindTexture(GL_TEXTURE_2D, *renderInstance->GArm()); // pbr specific

			glActiveTexture(GL_TEXTURE30);
			glBindTexture(GL_TEXTURE_2D, *renderInstance->GetAlternateScreenTexture());

			glActiveTexture(GL_TEXTURE31);
			glBindTexture(GL_TEXTURE_2D, *renderInstance->GetAlternateBloomBrightnessTexture());

			lightingPass = ResourceManager::GetInstance()->DeferredLightingPassPBR();
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);
			lightingPass->Use();
			lightingPass->setBool("useSSAO", ((renderOptions& RENDER_SSAO) != 0));
			LightManager::GetInstance()->SetShaderUniforms(lightingPass, activeCamera);
			ResourceManager::GetInstance()->DefaultPlane().DrawWithNoMaterial();

			// Skybox
			// ------
			if ((renderOptions & RENDER_SKYBOX) != 0 || (renderOptions & RENDER_ENVIRONMENT_MAP) != 0) {
				// Retrieve depth and stencil information from gBuffer
				glBindFramebuffer(GL_READ_FRAMEBUFFER, *renderInstance->GetGBuffer());
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
				if ((renderOptions & RENDER_ENVIRONMENT_MAP) != 0) {
					glBindTexture(GL_TEXTURE_CUBE_MAP, renderInstance->GetEnvironmentMap()->cubemapID);
				}
				else if ((renderOptions & RENDER_SKYBOX) != 0) {
					glBindTexture(GL_TEXTURE_CUBE_MAP, renderInstance->GetSkybox()->id);
				}
				ResourceManager::GetInstance()->DefaultCube().DrawWithNoMaterial();
				glCullFace(GL_BACK);
				glDepthFunc(GL_LESS);

				glBindBuffer(GL_UNIFORM_BUFFER, ResourceManager::GetInstance()->CommonUniforms());
				glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(activeCamera->GetViewMatrix()));
				glBindBuffer(GL_UNIFORM_BUFFER, 0);
			}

			// Transparency using forward rendering
			// ------------------------------------
			LightManager::GetInstance()->SetShaderUniforms(ResourceManager::GetInstance()->DefaultLitShader(), activeCamera);
			LightManager::GetInstance()->SetShaderUniforms(ResourceManager::GetInstance()->DefaultLitPBR(), activeCamera);
			glEnable(GL_BLEND);
			renderSystem->DrawTransparentGeometry(true);
			glDisable(GL_BLEND);

			// Particle render using forward rendering
			// ---------------------------------------
			if ((renderOptions & RENDER_PARTICLES) != 0) {
				ForwardParticleRenderStep();
			}

			// Bloom
			// -----
			BloomStep();

			// Advance bloom
			// -------------
			AdvancedBloomStep();

			// Post Processing
			// ---------------
			if ((renderOptions & RENDER_TONEMAPPING) != 0) {
				// HDR Tonemapping
				glViewport(0, 0, screenWidth, screenHeight);
				glBindFramebuffer(GL_FRAMEBUFFER, *renderInstance->GetTexturedFBO());
				glDrawBuffer(GL_COLOR_ATTACHMENT0);

				Shader* hdrShader = ResourceManager::GetInstance()->HDRTonemappingShader();
				hdrShader->Use();
				hdrShader->setFloat("gamma", renderInstance->GetRenderParams()->GetGamma());
				hdrShader->setFloat("exposure", renderInstance->GetRenderParams()->GetExposure());
				hdrShader->setBool("bloom", (renderOptions & RENDER_BLOOM) != 0);

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
			}

			// Final post process output
			glViewport(0, 0, screenWidth, screenHeight);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClear(GL_COLOR_BUFFER_BIT);

			Shader* screenQuadShader = ResourceManager::GetInstance()->ScreenQuadShader();
			screenQuadShader->Use();

			screenQuadShader->setUInt("postProcess", renderSystem->GetPostProcess());
			screenQuadShader->setFloat("postProcessStrength", renderInstance->GetRenderParams()->GetPostProcessStrength());

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

			// UI Render
			UIRenderStep();
		}
	}
}