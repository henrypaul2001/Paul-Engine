#include "ForwardPipeline.h"
#include "RenderManager.h"
namespace Engine {
	void ForwardPipeline::Run(EntityManagerNew* ecs, LightManager* lightManager, CollisionManager* collisionManager, Camera* activeCamera)
	{
		SCOPE_TIMER("ForwardPipeline::Run");
		RenderPipeline::Run(ecs, lightManager, collisionManager, activeCamera);

		RunShadowMapSteps();

		// render scene to textured framebuffer
		SceneRenderStep();
		BloomStep(*renderInstance->GetBloomBrightnessTexture());
		AdvancedBloomStep(*renderInstance->GetScreenTexture());
		ScreenTextureStep();
		UIRenderStep();
	}

	void ForwardPipeline::SceneRenderStep()
	{
		SCOPE_TIMER("ForwardPipeline::SceneRenderStep");
		const RenderOptions& renderOptions = renderInstance->GetRenderParams()->GetRenderOptions();
		glViewport(0, 0, screenWidth, screenHeight);

		// Render to textured framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, *renderInstance->GetTexturedFBO());
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *renderInstance->GetScreenTexture(), 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, *renderInstance->GetBloomBrightnessTexture(), 0);

		GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, drawBuffers);

		Camera* activeCamera = renderSystem.GetActiveCamera();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		// If frustum culling active
		renderSystem.RenderMeshes(SystemFrustumCulling::culledMeshList);

		// Render skybox
		if ((renderOptions & RENDER_SKYBOX) != 0 || (renderOptions & RENDER_ENVIRONMENT_MAP) != 0) {
			Shader* skyShader = resources->SkyboxShader();
			skyShader->Use();

			glBindBuffer(GL_UNIFORM_BUFFER, resources->CommonUniforms());
			glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(glm::mat4(glm::mat3(activeCamera->GetViewMatrix()))));
			glBindBuffer(GL_UNIFORM_BUFFER, 0);

			glDepthFunc(GL_LEQUAL);
			glCullFace(GL_FRONT);

			glDrawBuffer(GL_COLOR_ATTACHMENT0);

			glActiveTexture(GL_TEXTURE0);
			if ((renderOptions & RENDER_ENVIRONMENT_MAP) != 0) {
				glBindTexture(GL_TEXTURE_CUBE_MAP, renderInstance->GetEnvironmentMap()->cubemapID);
			}
			else if ((renderOptions & RENDER_SKYBOX) != 0) {
				glBindTexture(GL_TEXTURE_CUBE_MAP, renderInstance->GetSkybox()->id);
			}
			resources->DefaultCube().DrawWithNoMaterial();
			glCullFace(GL_BACK);
			glDepthFunc(GL_LESS);

			glBindBuffer(GL_UNIFORM_BUFFER, resources->CommonUniforms());
			glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(activeCamera->GetViewMatrix()));
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}

		// Render debug colliders
		// ----------------------
		if ((renderOptions & RENDER_GEOMETRY_COLLIDERS) != 0) {
			DebugCollidersStep();
		}

		// Render transparent objects
		renderSystem.RenderMeshes(SystemRender::transparentMeshes, true, false);
		renderSystem.AfterAction();

		if ((renderOptions & RENDER_PARTICLES) != 0) {
			// Render particles
			ForwardParticleRenderStep();
		}
	}

	void ForwardPipeline::ScreenTextureStep()
	{
		SCOPE_TIMER("ForwardPipeline::ScreenTextureStep");
		const RenderOptions& renderOptions = renderInstance->GetRenderParams()->GetRenderOptions();
		if ((renderOptions & RENDER_TONEMAPPING) != 0) {
			// HDR tonemapping step
			glViewport(0, 0, screenWidth, screenHeight);
			glBindFramebuffer(GL_FRAMEBUFFER, *renderInstance->GetTexturedFBO());
			glDrawBuffer(GL_COLOR_ATTACHMENT0);

			Shader* hdrShader = resources->HDRTonemappingShader();
			hdrShader->Use();
			hdrShader->setFloat("gamma", renderInstance->GetRenderParams()->GetGamma());
			hdrShader->setFloat("exposure", renderInstance->GetRenderParams()->GetExposure());

			hdrShader->setBool("bloom", (renderInstance->GetRenderParams()->GetRenderOptions() & RENDER_BLOOM) != 0);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, *renderInstance->GetScreenTexture());
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, *renderInstance->GetBloomPingPongColourBuffer(finalBloomTexture));
			glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			glDisable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);
			glDisable(GL_STENCIL_TEST);
			resources->DefaultPlane().DrawWithNoMaterial();
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);
			glEnable(GL_STENCIL_TEST);
		}

		// Post processing step
		glViewport(0, 0, screenWidth, screenHeight);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT);

		Shader* screenQuadShader = resources->ScreenQuadShader();
		screenQuadShader->Use();

		screenQuadShader->setUInt("postProcess", renderSystem.GetPostProcess());
		screenQuadShader->setFloat("postProcessStrength", renderInstance->GetRenderParams()->GetPostProcessStrength());

		if (renderSystem.GetPostProcess() == CUSTOM_KERNEL) {
			screenQuadShader->setFloat("customKernel[0]", renderSystem.PostProcessKernel[0]);
			screenQuadShader->setFloat("customKernel[1]", renderSystem.PostProcessKernel[1]);
			screenQuadShader->setFloat("customKernel[2]", renderSystem.PostProcessKernel[2]);
			screenQuadShader->setFloat("customKernel[3]", renderSystem.PostProcessKernel[3]);
			screenQuadShader->setFloat("customKernel[4]", renderSystem.PostProcessKernel[4]);
			screenQuadShader->setFloat("customKernel[5]", renderSystem.PostProcessKernel[5]);
			screenQuadShader->setFloat("customKernel[6]", renderSystem.PostProcessKernel[6]);
			screenQuadShader->setFloat("customKernel[7]", renderSystem.PostProcessKernel[7]);
			screenQuadShader->setFloat("customKernel[8]", renderSystem.PostProcessKernel[8]);
		}

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, *renderInstance->GetScreenTexture());
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		resources->DefaultPlane().DrawWithNoMaterial();
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
	}
}