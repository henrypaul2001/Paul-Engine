#include "SystemReflectionBaking.h"
#include "RenderManager.h"
#include "ComponentAnimator.h"
namespace Engine {
	void SystemReflectionBaking::Run(EntityManagerNew* ecs, LightManager* lightManager, const bool discardUnfilteredCapture)
	{
		SCOPE_TIMER("SystemReflectionBaking::Run");
		this->ecs = ecs;
		this->lightManager = lightManager;
		
		// Capture scene and process capture
		// ---------------------------------
		std::cout << "SYSTEMREFLECTIONBAKING::Baking reflection probes" << std::endl;
		ResourceManager* resources = ResourceManager::GetInstance();
		RenderManager* renderManager = RenderManager::GetInstance();
		const RenderOptions& renderOptions = renderManager->GetRenderParams()->GetRenderOptions();

		const unsigned int reflectionFBO = renderManager->GetCubemapFBO();
		const unsigned int depthBuffer = renderManager->GetCubemapDepthBuffer();

		unsigned int width, height;
		const std::vector<ReflectionProbe*>& probes = renderManager->GetBakedData().GetReflectionProbes();
		const int numProbes = probes.size();
		for (int i = 0; i < numProbes; i++) {
			// error check
			const int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if (status != GL_FRAMEBUFFER_COMPLETE) {
				std::cout << "ERROR::RENDERMANAGER::Cubemap FBO incomplete, status: 0x\%x\n" << status << std::endl;
			}

			ReflectionProbe* probe = probes[i];
			std::cout << "        - Baking probe " << i + 1 << " / " << numProbes << std::endl;

			// Capture scene
			width = probe->GetFaceWidth();
			height = probe->GetFaceHeight();
			const unsigned int cubemapID = probe->GetCubemapTextureID();

			currentViewPos = probe->GetWorldPosition();

			// Set up projections for each cubemap face
			currentProjection = glm::perspective(glm::radians(90.0f), 1.0f, probe->GetNearClip(), probe->GetFarClip());
			const glm::mat4 captureViews[] = {
				glm::lookAt(currentViewPos, currentViewPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
				glm::lookAt(currentViewPos, currentViewPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
				glm::lookAt(currentViewPos, currentViewPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
				glm::lookAt(currentViewPos, currentViewPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
				glm::lookAt(currentViewPos, currentViewPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
				glm::lookAt(currentViewPos, currentViewPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
			};

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			// Resize depth texture buffer
			glBindTexture(GL_TEXTURE_CUBE_MAP, depthBuffer);
			for (unsigned int j = 0; j < 6; j++) {
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
			}
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

			glBindFramebuffer(GL_FRAMEBUFFER, reflectionFBO);

			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
			glBindTexture(GL_TEXTURE_2D, 0);

			glViewport(0, 0, width, height);
			for (unsigned int j = 0; j < 6; ++j)
			{
				currentView = captureViews[j];
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, cubemapID, 0);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, depthBuffer, 0);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				// Render scene
				View<ComponentTransform, ComponentGeometry> geometryView = ecs->View<ComponentTransform, ComponentGeometry>();
				geometryView.ForEach(std::function<void(const unsigned int, ComponentTransform&, ComponentGeometry&)>(std::bind(&SystemReflectionBaking::OnAction, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));

				if (probe->GetRenderSkybox()) {
					// Render skybox
					if ((renderOptions & RENDER_SKYBOX) != 0 || (renderOptions & RENDER_ENVIRONMENT_MAP) != 0) {
						Shader* skyShader = resources->SkyboxShader();
						skyShader->Use();

						glBindBuffer(GL_UNIFORM_BUFFER, resources->CommonUniforms());
						glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(currentProjection));
						glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(glm::mat4(glm::mat3(captureViews[j]))));
						glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), sizeof(glm::vec3), glm::value_ptr(probe->GetWorldPosition()));
						glBindBuffer(GL_UNIFORM_BUFFER, 0);

						glDepthFunc(GL_LEQUAL);
						glCullFace(GL_FRONT);

						glDrawBuffer(GL_COLOR_ATTACHMENT0);

						glActiveTexture(GL_TEXTURE0);
						if ((renderOptions & RENDER_ENVIRONMENT_MAP) != 0) {
							glBindTexture(GL_TEXTURE_CUBE_MAP, renderManager->GetEnvironmentMap()->cubemapID);
						}
						else if ((renderOptions & RENDER_SKYBOX) != 0) {
							glBindTexture(GL_TEXTURE_CUBE_MAP, renderManager->GetSkybox()->id);
						}
						resources->DefaultCube().DrawWithNoMaterial();
						glCullFace(GL_BACK);
						glDepthFunc(GL_LESS);

						glBindBuffer(GL_UNIFORM_BUFFER, resources->CommonUniforms());
						glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(activeCamera->GetProjection()));
						glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(activeCamera->GetViewMatrix()));
						glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), sizeof(glm::vec3), glm::value_ptr(activeCamera->GetPosition()));
						glBindBuffer(GL_UNIFORM_BUFFER, 0);
					}
				}

				glEnable(GL_BLEND);
				DrawTransparentGeometry();
				glDisable(GL_BLEND);
			}

			// Process capture
			glDisable(GL_CULL_FACE);

			ConvoluteEnvironmentMap(probe);
			PrefilterMap(probe);

			probe->SetDiscardUnfilteredCapture(discardUnfilteredCapture);
			if (discardUnfilteredCapture) {
				glDeleteTextures(1, &cubemapID);
			}

			glEnable(GL_CULL_FACE);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, renderManager->ScreenWidth(), renderManager->ScreenHeight());

		// Write to file
		// -------------
		renderManager->GetBakedData().WriteReflectionProbesToFile();
	}

	void SystemReflectionBaking::ConvoluteEnvironmentMap(const ReflectionProbe* probe)
	{
		RenderManager* renderManager = RenderManager::GetInstance();
		ResourceManager* resourceManager = ResourceManager::GetInstance();

		const unsigned int cubeCaptureFBO = *renderManager->GetHDRCubeCaptureFBO();
		const unsigned int cubeCaptureRBO = *renderManager->GetHDRCubeCaptureRBO();
		const unsigned int depthBuffer = renderManager->GetCubemapDepthBuffer();

		const unsigned int envCubemap = probe->GetCubemapTextureID();
		const unsigned int irradianceMapArray = renderManager->GetBakedData().GetProbeIrradianceMapArray();

		const unsigned int probeID = probe->GetFileID();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		// Resize depth texture buffer
		glBindTexture(GL_TEXTURE_CUBE_MAP, depthBuffer);
		for (unsigned int i = 0; i < 6; i++) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT24, 32, 32, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
		}
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		glBindFramebuffer(GL_FRAMEBUFFER, cubeCaptureFBO);
		glBindRenderbuffer(GL_RENDERBUFFER, cubeCaptureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

		// Set up projections for each cubemap face
		const glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		const glm::mat4 captureViews[] = {
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
		};

		// Set up irradiance shader uniforms and bind env cupemap
		Shader* irradianceShader = resourceManager->CreateIrradianceShader();
		irradianceShader->Use();
		irradianceShader->setInt("environmentMap", 0);
		irradianceShader->setMat4("projection", captureProjection);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

		glViewport(0, 0, 32, 32);
		glBindFramebuffer(GL_FRAMEBUFFER, cubeCaptureFBO);
		for (unsigned int i = 0; i < 6; ++i)
		{
			irradianceShader->setMat4("view", captureViews[i]);
			glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, irradianceMapArray, 0, probeID * 6 + i);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, depthBuffer, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			resourceManager->DefaultCube().DrawWithNoMaterial();
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}

	void SystemReflectionBaking::PrefilterMap(const ReflectionProbe* probe)
	{
		RenderManager* renderManager = RenderManager::GetInstance();
		ResourceManager* resourceManager = ResourceManager::GetInstance();

		const unsigned int envCubemap = probe->GetCubemapTextureID();
		const unsigned int prefilterMapArray = renderManager->GetBakedData().GetProbePrefilterMapArray();

		const unsigned int cubeCaptureFBO = *renderManager->GetHDRCubeCaptureFBO();
		const unsigned int cubeCaptureRBO = *renderManager->GetHDRCubeCaptureRBO();
		const unsigned int depthBuffer = renderManager->GetCubemapDepthBuffer();

		const unsigned int faceWidth = probe->GetFaceWidth();
		const unsigned int faceHeight = probe->GetFaceHeight();

		const unsigned int probeID = probe->GetFileID();

		// Set up projections for each cubemap face
		const glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		const glm::mat4 captureViews[] = {
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
		};

		// Capture prefilter mipmap levels
		// -------------------------------
		Shader* prefilterShader = resourceManager->CreatePrefilterShader();
		prefilterShader->Use();
		prefilterShader->setInt("environmentMap", 0);
		prefilterShader->setMat4("projection", captureProjection);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		glBindFramebuffer(GL_FRAMEBUFFER, cubeCaptureFBO);

		unsigned int maxMipLevels = 5;

		glDisable(GL_DEPTH_TEST);

		for (unsigned int mip = 0; mip < maxMipLevels; mip++) {
			// resize framebuffer
			const unsigned int mipWidth = (faceWidth / 2) * std::pow(0.5, mip);
			const unsigned int mipHeight = (faceHeight / 2) * std::pow(0.5, mip);

			glBindRenderbuffer(GL_RENDERBUFFER, cubeCaptureRBO);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, cubeCaptureRBO);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			// Resize depth texture buffer
			glBindTexture(GL_TEXTURE_CUBE_MAP, depthBuffer);
			for (unsigned int i = 0; i < 6; i++) {
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT24, mipWidth, mipHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
			}
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
			glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
			glBindFramebuffer(GL_FRAMEBUFFER, cubeCaptureFBO);

			glViewport(0, 0, mipWidth, mipHeight);

			const float roughness = (float)mip / (float)(maxMipLevels - 1);
			prefilterShader->setFloat("roughness", roughness);
			prefilterShader->setUInt("faceWidth", faceWidth);
			prefilterShader->setUInt("faceHeight", faceHeight);
			for (unsigned int i = 0; i < 6; i++) {
				prefilterShader->setMat4("view", captureViews[i]);
				glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, prefilterMapArray, mip, probeID * 6 + i);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, depthBuffer, 0);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				resourceManager->DefaultCube().DrawWithNoMaterial();
			}
		}

		glEnable(GL_DEPTH_TEST);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}

	void SystemReflectionBaking::OnAction(const unsigned int entityID, ComponentTransform& transform, ComponentGeometry& geometry)
	{
		SCOPE_TIMER("SystemReflectionBaking::OnAction");
		if (geometry.IsIncludedInReflectionProbes()) {
			ResourceManager* resources = ResourceManager::GetInstance();
			Shader* reflectionShader;
			if (geometry.PBR()) { reflectionShader = resources->ReflectionProbeBakingShaderPBR(); }
			else { reflectionShader = resources->ReflectionProbeBakingShader(); }
			reflectionShader->Use();

			lightManager->SetShaderUniforms(*ecs, reflectionShader, nullptr);

			reflectionShader->setMat4("projection", currentProjection);
			reflectionShader->setVec3("viewPos", currentViewPos);
			reflectionShader->setMat4("view", currentView);

			// setup shader uniforms
			const glm::mat4& model = transform.GetWorldModelMatrix();
			reflectionShader->setMat4("model", model);
			reflectionShader->setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
			reflectionShader->setBool("instanced", false);
			//if (geometry->Instanced()) { geometry->BufferInstanceTransforms(); }
			reflectionShader->setVec2("textureScale", geometry.GetTextureScale());
			reflectionShader->setBool("hasBones", false);
			reflectionShader->setBool("OpaqueRenderPass", true);

			// Bones
			if (geometry.GetModel()->HasBones()) {
				ComponentAnimator* animator = ecs->GetComponent<ComponentAnimator>(entityID);
				if (animator) {
					reflectionShader->setBool("hasBones", true);
					const std::vector<glm::mat4>& transforms = animator->GetFinalBonesMatrices();
					for (int i = 0; i < transforms.size(); i++) {
						reflectionShader->setMat4("boneTransforms[" + std::to_string(i) + "]", transforms[i]);
					}
				}
			}

			// Apply face culling
			if (geometry.Cull_Face()) { glEnable(GL_CULL_FACE); }
			else { glDisable(GL_CULL_FACE); }

			if (geometry.Cull_Type() == GL_BACK) {
				///glCullFace(GL_FRONT);
				glCullFace(GL_BACK);
			}
			else if (geometry.Cull_Type() == GL_FRONT) {
				//glCullFace(GL_BACK);
				glCullFace(GL_FRONT);
			}

			// Draw geometry
			//geometry.GetModel()->Draw(*reflectionShader, geometry->NumInstances(), geometry->GetInstanceVAOs());
			geometry.GetModel()->Draw(*reflectionShader, 0, {});

			if (geometry.GetModel()->ContainsTransparentMeshes()) {
				float distanceToCamera = glm::length(activeCamera->GetPosition() - transform.GetWorldPosition());

				if (transparentGeometry.find(distanceToCamera) != transparentGeometry.end()) {
					// Distance already exists, increment slightly
					distanceToCamera += 0.00001f;
				}
				transparentGeometry[distanceToCamera] = std::make_pair(&geometry, entityID);
			}
		}
	}

	void SystemReflectionBaking::DrawTransparentGeometry()
	{
		ResourceManager* resources = ResourceManager::GetInstance();
		// Geometry is already sorted in ascending order
		for (std::map<float, std::pair<ComponentGeometry*, unsigned int>>::reverse_iterator it = transparentGeometry.rbegin(); it != transparentGeometry.rend(); ++it) {
			ComponentGeometry* geometry = it->second.first;
			const unsigned int entityID = it->second.second;
			const ComponentTransform* transform = ecs->GetComponent<ComponentTransform>(entityID);

			if (geometry->IsIncludedInReflectionProbes()) {
				Shader* reflectionShader;
				if (geometry->PBR()) { reflectionShader = resources->ReflectionProbeBakingShaderPBR(); }
				else { reflectionShader = resources->ReflectionProbeBakingShader(); }
				reflectionShader->Use();

				// setup shader uniforms
				const glm::mat4& model = transform->GetWorldModelMatrix();
				reflectionShader->setMat4("model", model);
				reflectionShader->setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
				reflectionShader->setBool("instanced", false);
				//if (geometry->Instanced()) { geometry->BufferInstanceTransforms(); }
				reflectionShader->setVec2("textureScale", geometry->GetTextureScale());
				reflectionShader->setBool("hasBones", false);
				reflectionShader->setBool("OpaqueRenderPass", false);

				// Bones
				if (geometry->GetModel()->HasBones()) {
					ComponentAnimator* animator = ecs->GetComponent<ComponentAnimator>(entityID);
					if (animator) {
						reflectionShader->setBool("hasBones", true);
						const std::vector<glm::mat4>& transforms = animator->GetFinalBonesMatrices();
						for (int i = 0; i < transforms.size(); i++) {
							reflectionShader->setMat4("boneTransforms[" + std::to_string(i) + "]", transforms[i]);
						}
					}
				}

				// Apply face culling
				if (geometry->Cull_Face()) { glEnable(GL_CULL_FACE); }
				else { glDisable(GL_CULL_FACE); }

				if (geometry->Cull_Type() == GL_BACK) {
					///glCullFace(GL_FRONT);
					glCullFace(GL_BACK);
				}
				else if (geometry->Cull_Type() == GL_FRONT) {
					//glCullFace(GL_BACK);
					glCullFace(GL_FRONT);
				}

				//geometry->GetModel()->DrawTransparentMeshes(*reflectionShader, geometry->NumInstances(), geometry->GetInstanceVAOs());
				geometry->GetModel()->DrawTransparentMeshes(*reflectionShader, 0, {});
			}
		}

		transparentGeometry.clear();
	}
}