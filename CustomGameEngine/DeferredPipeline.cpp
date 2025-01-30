#include "DeferredPipeline.h"
#include "RenderManager.h"

namespace Engine {
	void DeferredPipeline::Run(EntityManager* ecs, LightManager* lightManager, CollisionManager* collisionManager, Camera* activeCamera)
	{
		SCOPE_TIMER("DeferredPipeline::Run");
		RenderPipeline::Run(ecs, lightManager, collisionManager, activeCamera);
		const RenderParams& renderParams = *renderInstance->GetRenderParams();
		const RenderOptions& renderOptions = renderParams.GetRenderOptions();
		activeScreenTexture = *renderInstance->GetScreenTexture();
		alternateScreenTexture = *renderInstance->GetAlternateScreenTexture();

		activeBloomTexture = *renderInstance->GetBloomBrightnessTexture();
		alternateBloomTexture = *renderInstance->GetAlternateBloomBrightnessTexture();

		// shadow map steps
		RunShadowMapSteps();

		// Geometry pass
		// -------------
		//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		{
			SCOPE_TIMER("DeferredPipeline::Geometry Pass");
			glViewport(0, 0, screenWidth, screenHeight);
			glBindFramebuffer(GL_FRAMEBUFFER, *renderInstance->GetGBuffer());
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glDisable(GL_BLEND);
			renderSystem.RenderMeshes(SystemFrustumCulling::culledMeshList);
			renderSystem.AfterAction();
		}

		// SSAO Pass
		// ---------
		if ((renderOptions & RENDER_SSAO) != 0) {
			SCOPE_TIMER("DeferredPipeline::SSAO Pass");
			// SSAO Texture
			glBindFramebuffer(GL_FRAMEBUFFER, *renderInstance->GetSSAOFBO());
			glClear(GL_COLOR_BUFFER_BIT);
			Shader* ssaoShader = resources->SSAOShader();
			ssaoShader->Use();
			ssaoShader->setInt("scr_width", screenWidth);
			ssaoShader->setInt("scr_height", screenHeight);

			int samples = renderParams.GetSSAOSamples();
			const float radius = renderParams.GetSSAORadius();
			const float bias = renderParams.GetSSAOBias();

			ssaoShader->setInt("kernelSize", samples);
			ssaoShader->setFloat("radius", radius);
			ssaoShader->setFloat("bias", bias);

			if (samples > 64) { samples = 64; }
			else if (samples < 0) { samples = 0; }

			// Send kernel + rotation
			std::vector<glm::vec3*> ssaoKernel = renderInstance->SSAOKernel(); // pointers to vec3s ?!?!?!?!? why? TODO
			for (unsigned int i = 0; i < samples; i++) {
				ssaoShader->setVec3("samples[" + std::to_string(i) + "]", *ssaoKernel[i]);
			}

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, *renderInstance->GPosition());
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, *renderInstance->GNormal());
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, *renderInstance->SSAONoiseTexture());
			resources->DefaultPlane().DrawWithNoMaterial();

			// Blur SSAO texture to remove noise
			glBindFramebuffer(GL_FRAMEBUFFER, *renderInstance->GetSSAOBlurFBO());
			glClear(GL_COLOR_BUFFER_BIT);
			Shader* ssaoBlur = resources->SSABlur();
			ssaoBlur->Use();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, *renderInstance->SSAOColour());
			resources->DefaultPlane().DrawWithNoMaterial();
		}

		// SSR UV Pass
		// -----------
		if ((renderOptions & RENDER_SSR) != 0) {
			SCOPE_TIMER("DeferredPipeline::SSR UV Pass");
			glViewport(0, 0, screenWidth, screenHeight);
			glBindFramebuffer(GL_FRAMEBUFFER, *renderInstance->GetTexturedFBO());

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderInstance->GetSSRUVMap(), 0);

			glDrawBuffer(GL_COLOR_ATTACHMENT0);
			glClear(GL_COLOR_BUFFER_BIT);
			glDisable(GL_DEPTH_TEST);

			Shader* ssrUVShader = resources->ScreenSpaceReflectionUVMappingShader();
			ssrUVShader->Use();

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, *renderInstance->GViewSpacePos());

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, *renderInstance->GNormal());

			ssrUVShader->setFloat("rayAcceleration", renderParams.GetSSRRayAcceleration());
			ssrUVShader->setFloat("rayStep", renderParams.GetSSRRayStep());
			ssrUVShader->setUInt("maxSteps", renderParams.GetSSRMaxSteps());
			ssrUVShader->setFloat("maxDistance", renderParams.GetSSRMaxDistance());
			ssrUVShader->setFloat("rayThickness", renderParams.GetSSRRayThickness());
			ssrUVShader->setUInt("numBinarySearchSteps", renderParams.GetSSRNumBinarySearchSteps());

			resources->DefaultPlane().DrawWithNoMaterial();
		}

		// Lighting pass
		// -------------

		// Two lighting passes, first, non pbr will light all non pbr pixels
		// Second pass will light all pbr pixels
		{
			SCOPE_TIMER("DeferredPipeline::Lighting Pass");
			glViewport(0, 0, screenWidth, screenHeight);
			glBindFramebuffer(GL_FRAMEBUFFER, *renderInstance->GetTexturedFBO());
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, activeScreenTexture, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, activeBloomTexture, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			const GLenum buffers[]{ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
			glDrawBuffers(2, buffers);

			glActiveTexture(GL_TEXTURE0 + textureLookups->at("gPosition"));
			glBindTexture(GL_TEXTURE_2D, *renderInstance->GPosition());
			glActiveTexture(GL_TEXTURE0 + textureLookups->at("gNormal"));
			glBindTexture(GL_TEXTURE_2D, *renderInstance->GNormal());
			glActiveTexture(GL_TEXTURE0 + textureLookups->at("gAlbedo"));
			glBindTexture(GL_TEXTURE_2D, *renderInstance->GAlbedo());

			glActiveTexture(GL_TEXTURE0 + textureLookups->at("gSpecular"));
			glBindTexture(GL_TEXTURE_2D, *renderInstance->GSpecular()); // non pbr specific

			glActiveTexture(GL_TEXTURE0 + textureLookups->at("gPBRFLAG"));
			glBindTexture(GL_TEXTURE_2D, *renderInstance->GPBRFLAG());

			glActiveTexture(GL_TEXTURE0 + textureLookups->at("SSAO"));
			glBindTexture(GL_TEXTURE_2D, *renderInstance->SSAOBlurColour());

			// First pass
			Shader* lightingPass = resources->DeferredLightingPass();
			glDisable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);
			lightingPass->Use();
			lightingPass->setBool("useSSAO", ((renderOptions & RENDER_SSAO) != 0));
			lightManager->SetShaderUniforms(*ecs, lightingPass, activeCamera);
			resources->DefaultPlane().DrawWithNoMaterial();

			// Second pass (pbr)
			glBindFramebuffer(GL_FRAMEBUFFER, *renderInstance->GetTexturedFBO());

			SwapScreenTextures();
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, activeScreenTexture, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, activeBloomTexture, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glActiveTexture(GL_TEXTURE0 + textureLookups->at("gArm"));
			glBindTexture(GL_TEXTURE_2D, *renderInstance->GArm()); // pbr specific

			glActiveTexture(GL_TEXTURE30);
			glBindTexture(GL_TEXTURE_2D, alternateScreenTexture);

			glActiveTexture(GL_TEXTURE31);
			glBindTexture(GL_TEXTURE_2D, alternateBloomTexture);

			lightingPass = resources->DeferredLightingPassPBR();
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);
			lightingPass->Use();
			lightingPass->setBool("useSSAO", ((renderOptions & RENDER_SSAO) != 0));
			lightManager->SetShaderUniforms(*ecs, lightingPass, activeCamera);
			resources->DefaultPlane().DrawWithNoMaterial();
		}

		// SSR Convert UV to Reflection Map
		// --------------------------------
		if ((renderOptions & RENDER_SSR) != 0) {
			SCOPE_TIMER("DeferredPipeline::SSR UV to Reflection Map");
			glViewport(0, 0, screenWidth, screenHeight);
			glBindFramebuffer(GL_FRAMEBUFFER, *renderInstance->GetTexturedFBO());

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderInstance->GetSSRReflectionMap(), 0);

			glDrawBuffer(GL_COLOR_ATTACHMENT0);
			glClear(GL_COLOR_BUFFER_BIT);
			glDisable(GL_DEPTH_TEST);

			Shader* ssrReflectionMapShader = resources->SSRUVMapToReflectionMap();
			ssrReflectionMapShader->Use();

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, activeScreenTexture);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, renderInstance->GetSSRUVMap());

			resources->DefaultPlane().DrawWithNoMaterial();
		}

		// SSR Combine pass
		// ----------------
		if ((renderOptions & RENDER_SSR) != 0) {
			SCOPE_TIMER("DeferredPipeline::SSR Combine Pass");
			glViewport(0, 0, screenWidth, screenHeight);
			glBindFramebuffer(GL_FRAMEBUFFER, *renderInstance->GetTexturedFBO());

			SwapScreenTextures();
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, activeScreenTexture, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, activeBloomTexture, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			const GLenum buffers[]{ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
			glDrawBuffers(2, buffers);

			glActiveTexture(GL_TEXTURE0 + textureLookups->at("gPosition"));
			glBindTexture(GL_TEXTURE_2D, *renderInstance->GPosition());
			glActiveTexture(GL_TEXTURE0 + textureLookups->at("gNormal"));
			glBindTexture(GL_TEXTURE_2D, *renderInstance->GNormal());
			glActiveTexture(GL_TEXTURE0 + textureLookups->at("gAlbedo"));
			glBindTexture(GL_TEXTURE_2D, *renderInstance->GAlbedo());
			glActiveTexture(GL_TEXTURE0 + textureLookups->at("gPBRFLAG"));
			glBindTexture(GL_TEXTURE_2D, *renderInstance->GPBRFLAG());
			glActiveTexture(GL_TEXTURE0 + textureLookups->at("gArm"));
			glBindTexture(GL_TEXTURE_2D, *renderInstance->GArm());
			glActiveTexture(GL_TEXTURE0 + textureLookups->at("lightingPass"));
			glBindTexture(GL_TEXTURE_2D, alternateScreenTexture);
			glActiveTexture(GL_TEXTURE0 + textureLookups->at("brdfLUT"));
			glBindTexture(GL_TEXTURE_2D, renderInstance->GetGlobalBRDF_LUT());
			glActiveTexture(GL_TEXTURE0 + textureLookups->at("gSpecular"));
			glBindTexture(GL_TEXTURE_2D, *renderInstance->GSpecular());

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, renderInstance->GetSSRUVMap());
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, renderInstance->GetSSRReflectionMap());

			Shader* ssrCombineShader = resources->SSRCombineShader();
			ssrCombineShader->Use();
			ssrCombineShader->setFloat("BloomThreshold", renderInstance->GetRenderParams()->GetBloomThreshold());
			resources->DefaultPlane().DrawWithNoMaterial();
		}

		// IBL Pass
		// --------
		if ((renderOptions & RENDER_IBL) != 0) {
			SCOPE_TIMER("DeferredPipeline::IBL Pass");
			glViewport(0, 0, screenWidth, screenHeight);
			glBindFramebuffer(GL_FRAMEBUFFER, *renderInstance->GetTexturedFBO());

			SwapScreenTextures();
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, activeScreenTexture, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, activeBloomTexture, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			const GLenum buffers[]{ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
			glDrawBuffers(2, buffers);

			Shader* iblShader = resources->DeferredIBLPassPBR();
			iblShader->Use();

			glActiveTexture(GL_TEXTURE0 + 2);
			glBindTexture(GL_TEXTURE_2D, renderInstance->GetSSRUVMap());
			glActiveTexture(GL_TEXTURE0 + textureLookups->at("gPosition"));
			glBindTexture(GL_TEXTURE_2D, *renderInstance->GPosition());
			glActiveTexture(GL_TEXTURE0 + textureLookups->at("gNormal"));
			glBindTexture(GL_TEXTURE_2D, *renderInstance->GNormal());
			glActiveTexture(GL_TEXTURE0 + textureLookups->at("gAlbedo"));
			glBindTexture(GL_TEXTURE_2D, *renderInstance->GAlbedo());
			glActiveTexture(GL_TEXTURE0 + textureLookups->at("gPBRFLAG"));
			glBindTexture(GL_TEXTURE_2D, *renderInstance->GPBRFLAG());
			glActiveTexture(GL_TEXTURE0 + textureLookups->at("gArm"));
			glBindTexture(GL_TEXTURE_2D, *renderInstance->GArm());
			glActiveTexture(GL_TEXTURE0 + textureLookups->at("lightingPass"));
			glBindTexture(GL_TEXTURE_2D, alternateScreenTexture);
			glActiveTexture(GL_TEXTURE0 + textureLookups->at("SSAO"));
			glBindTexture(GL_TEXTURE_2D, *renderInstance->SSAOBlurColour());
			iblShader->setBool("useSSAO", ((renderOptions& RENDER_SSAO) != 0));
			iblShader->setBool("useSSR", ((renderOptions& RENDER_SSR) != 0));
			iblShader->setFloat("minIBLSSRBlend", renderParams.GetMinIBLSSRBlend());
			lightManager->SetShaderUniforms(*ecs, iblShader, activeCamera);
			resources->DefaultPlane().DrawWithNoMaterial();
		}

		// Skybox
		// ------
		if ((renderOptions & RENDER_SKYBOX) != 0 || (renderOptions & RENDER_ENVIRONMENT_MAP) != 0) {
			SCOPE_TIMER("DeferredPipeline::Skybox");
			// Retrieve depth and stencil information from gBuffer
			glBindFramebuffer(GL_READ_FRAMEBUFFER, *renderInstance->GetGBuffer());
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, *renderInstance->GetTexturedFBO());
			glBlitFramebuffer(0, 0, screenWidth, screenHeight, 0, 0, screenWidth, screenHeight, GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
			glBindFramebuffer(GL_FRAMEBUFFER, *renderInstance->GetTexturedFBO());
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, activeScreenTexture, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, activeBloomTexture, 0);

			Shader* skyShader = resources->SkyboxShader();
			skyShader->Use();

			glDrawBuffer(GL_COLOR_ATTACHMENT0);

			glBindBuffer(GL_UNIFORM_BUFFER, resources->CommonUniforms());
			glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(glm::mat4(glm::mat3(activeCamera->GetViewMatrix()))));
			glBindBuffer(GL_UNIFORM_BUFFER, 0);

			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LEQUAL);
			glCullFace(GL_FRONT);

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

		// Render debug colliders using forward rendering
		// ----------------------------------------------
		if ((renderOptions & RENDER_GEOMETRY_COLLIDERS) != 0) {
			DebugCollidersStep();
		}

		// Transparency using forward rendering
		// ------------------------------------
		{
			SCOPE_TIMER("DeferredPipeline::Transparency Pass");
			lightManager->SetShaderUniforms(*ecs, resources->DefaultLitShader(), activeCamera);
			lightManager->SetShaderUniforms(*ecs, resources->DefaultLitPBR(), activeCamera);
			glEnable(GL_BLEND);
			renderSystem.RenderMeshes(SystemRender::transparentMeshes, true, true);
			glDisable(GL_BLEND);
		}

		// Particle render using forward rendering
		// ---------------------------------------
		if ((renderOptions & RENDER_PARTICLES) != 0) {
			ForwardParticleRenderStep();
		}

		// Bloom
		// -----
		BloomStep(activeBloomTexture);

		// Advance bloom
		// -------------
		AdvancedBloomStep(activeScreenTexture);

		// Post Processing
		// ---------------
		if ((renderOptions & RENDER_TONEMAPPING) != 0) {
			SCOPE_TIMER("DeferredPipeline::Post Processing");
			// HDR Tonemapping
			glViewport(0, 0, screenWidth, screenHeight);
			glBindFramebuffer(GL_FRAMEBUFFER, *renderInstance->GetTexturedFBO());
			SwapScreenTextures();
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, activeScreenTexture, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, activeBloomTexture, 0);
			glDrawBuffer(GL_COLOR_ATTACHMENT0);

			Shader* hdrShader = resources->HDRTonemappingShader();
			hdrShader->Use();
			hdrShader->setFloat("gamma", renderInstance->GetRenderParams()->GetGamma());
			hdrShader->setFloat("exposure", renderInstance->GetRenderParams()->GetExposure());
			hdrShader->setBool("bloom", (renderOptions & RENDER_BLOOM) != 0);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, alternateScreenTexture);
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

		// Final post process output
		{
			SCOPE_TIMER("DeferredPipeline::Final Screen");
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
			glBindTexture(GL_TEXTURE_2D, activeScreenTexture);
			glDisable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);
			resources->DefaultPlane().DrawWithNoMaterial();
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);
		}

		// UI Render
		UIRenderStep();
	}
}