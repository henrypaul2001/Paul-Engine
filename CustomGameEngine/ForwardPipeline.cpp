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

		this->renderSystems = renderSystems;

		// shadow map steps
		if (shadowmapSystem != nullptr) {
			RunShadowMapSteps();
		}

		// render scene to textured framebuffer
		if (renderSystem != nullptr) {

			SceneRenderStep();

			BloomStep();

			ScreenTextureStep();

			UIRenderStep();
		}
	}

	void ForwardPipeline::SceneRenderStep()
	{
		RenderOptions renderOptions = renderInstance->GetRenderParams()->GetRenderOptions();
		glViewport(0, 0, screenWidth, screenHeight);

		// Render to textured framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, *renderInstance->GetTexturedFBO());

		GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, drawBuffers);

		Camera* activeCamera = renderSystem->GetActiveCamera();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		for (Entity* e : entities) {
			renderSystem->OnAction(e);
		}
		renderSystem->AfterAction();

		// Render skybox
		if ((renderOptions & RENDER_SKYBOX) != 0 || (renderOptions & RENDER_ENVIRONMENT_MAP) != 0) {
			Shader* skyShader = ResourceManager::GetInstance()->SkyboxShader();
			skyShader->Use();

			glBindBuffer(GL_UNIFORM_BUFFER, ResourceManager::GetInstance()->CommonUniforms());
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
			ResourceManager::GetInstance()->DefaultCube().DrawWithNoMaterial();
			glCullFace(GL_BACK);
			glDepthFunc(GL_LESS);

			glBindBuffer(GL_UNIFORM_BUFFER, ResourceManager::GetInstance()->CommonUniforms());
			glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(activeCamera->GetViewMatrix()));
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}

		// Render transparent objects
		renderSystem->DrawTransparentGeometry(false);

		if ((renderOptions & RENDER_PARTICLES) != 0) {
			// Render particles
			ForwardParticleRenderStep();
		}

		// Run any other render systems that may have been added
		for (System* s : renderSystems) {
			if (s->Name() != SYSTEM_RENDER && s->Name() != SYSTEM_UI_RENDER && s->Name() != SYSTEM_SHADOWMAP && s->Name() != SYSTEM_PARTICLE_RENDER) {
				for (Entity* e : entities) {
					s->OnAction(e);
				}
				s->AfterAction();
			}
		}
	}

	void ForwardPipeline::ScreenTextureStep()
	{
		RenderOptions renderOptions = renderInstance->GetRenderParams()->GetRenderOptions();
		if ((renderOptions & RENDER_TONEMAPPING) != 0) {
			// HDR tonemapping step
			glViewport(0, 0, screenWidth, screenHeight);
			glBindFramebuffer(GL_FRAMEBUFFER, *renderInstance->GetTexturedFBO());
			glDrawBuffer(GL_COLOR_ATTACHMENT0);

			Shader* hdrShader = ResourceManager::GetInstance()->HDRTonemappingShader();
			hdrShader->Use();
			hdrShader->setFloat("gamma", 1.2);
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
			ResourceManager::GetInstance()->DefaultPlane().DrawWithNoMaterial();
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);
			glEnable(GL_STENCIL_TEST);
		}

		// Post processing step
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