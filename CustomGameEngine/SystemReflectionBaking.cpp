#include "SystemReflectionBaking.h"
#include "RenderManager.h"
#include "LightManager.h"
namespace Engine {
	SystemReflectionBaking::SystemReflectionBaking()
	{

	}

	SystemReflectionBaking::~SystemReflectionBaking()
	{

	}

	void SystemReflectionBaking::Run(const std::vector<Entity*>& entityList, const bool discardUnfilteredCapture)
	{
		SCOPE_TIMER("SystemReflectionBaking::Run");
		// Capture scene and process capture
		// ---------------------------------
		std::cout << "SYSTEMREFLECTIONBAKING::Baking reflection probes" << std::endl;
		RenderManager* renderManager = RenderManager::GetInstance();
		RenderOptions renderOptions = renderManager->GetRenderParams()->GetRenderOptions();

		unsigned int reflectionFBO = renderManager->GetCubemapFBO();
		unsigned int depthBuffer = renderManager->GetCubemapDepthBuffer();

		unsigned int width, height;
		std::vector<ReflectionProbe*> probes = renderManager->GetBakedData().GetReflectionProbes();
		int numProbes = probes.size();
		for (int i = 0; i < numProbes; i++) {
			// error check
			int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if (status != GL_FRAMEBUFFER_COMPLETE) {
				std::cout << "ERROR::RENDERMANAGER::Cubemap FBO incomplete, status: 0x\%x\n" << status << std::endl;
			}

			ReflectionProbe* probe = probes[i];
			std::cout << "        - Baking probe " << i + 1 << " / " << numProbes << std::endl;

			// Capture scene
			width = probe->GetFaceWidth();
			height = probe->GetFaceHeight();
			unsigned int cubemapID = probe->GetCubemapTextureID();

			currentViewPos = probe->GetWorldPosition();

			// Set up projections for each cubemap face
			currentProjection = glm::perspective(glm::radians(90.0f), 1.0f, probe->GetNearClip(), probe->GetFarClip());
			glm::mat4 captureViews[] = {
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
				for (Entity* entity : entityList) {
					OnAction(entity);
				}

				if (probe->GetRenderSkybox()) {
					// Render skybox
					if ((renderOptions & RENDER_SKYBOX) != 0 || (renderOptions & RENDER_ENVIRONMENT_MAP) != 0) {
						Shader* skyShader = ResourceManager::GetInstance()->SkyboxShader();
						skyShader->Use();

						glBindBuffer(GL_UNIFORM_BUFFER, ResourceManager::GetInstance()->CommonUniforms());
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
						ResourceManager::GetInstance()->DefaultCube().DrawWithNoMaterial();
						glCullFace(GL_BACK);
						glDepthFunc(GL_LESS);

						glBindBuffer(GL_UNIFORM_BUFFER, ResourceManager::GetInstance()->CommonUniforms());
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

	void SystemReflectionBaking::ConvoluteEnvironmentMap(ReflectionProbe* probe)
	{
		RenderManager* renderManager = RenderManager::GetInstance();
		ResourceManager* resourceManager = ResourceManager::GetInstance();

		unsigned int cubeCaptureFBO = *renderManager->GetHDRCubeCaptureFBO();
		unsigned int cubeCaptureRBO = *renderManager->GetHDRCubeCaptureRBO();
		unsigned int depthBuffer = renderManager->GetCubemapDepthBuffer();

		unsigned int envCubemap = probe->GetCubemapTextureID();
		unsigned int irradianceMapArray = renderManager->GetBakedData().GetProbeIrradianceMapArray();

		unsigned int probeID = probe->GetFileID();

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
		glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		glm::mat4 captureViews[] = {
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

	void SystemReflectionBaking::PrefilterMap(ReflectionProbe* probe)
	{
		RenderManager* renderManager = RenderManager::GetInstance();
		ResourceManager* resourceManager = ResourceManager::GetInstance();

		unsigned int envCubemap = probe->GetCubemapTextureID();
		unsigned int prefilterMapArray = renderManager->GetBakedData().GetProbePrefilterMapArray();

		unsigned int cubeCaptureFBO = *renderManager->GetHDRCubeCaptureFBO();
		unsigned int cubeCaptureRBO = *renderManager->GetHDRCubeCaptureRBO();
		unsigned int depthBuffer = renderManager->GetCubemapDepthBuffer();

		unsigned int faceWidth = probe->GetFaceWidth();
		unsigned int faceHeight = probe->GetFaceHeight();

		unsigned int probeID = probe->GetFileID();

		// Set up projections for each cubemap face
		glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		glm::mat4 captureViews[] = {
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
			unsigned int mipWidth = (faceWidth / 2) * std::pow(0.5, mip);
			unsigned int mipHeight = (faceHeight / 2) * std::pow(0.5, mip);

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

			float roughness = (float)mip / (float)(maxMipLevels - 1);
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

	void SystemReflectionBaking::OnAction(Entity* entity)
	{
		SCOPE_TIMER("SystemReflectionBaking::OnAction");
		if ((entity->Mask() & MASK) == MASK) {
			ComponentTransform* transform = entity->GetTransformComponent();
			ComponentGeometry* geometry = entity->GetGeometryComponent();

			Draw(transform, geometry);
		}
	}

	void SystemReflectionBaking::AfterAction()
	{

	}

	void SystemReflectionBaking::Draw(ComponentTransform* transform, ComponentGeometry* geometry)
	{
		if (geometry->IsIncludedInReflectionProbes()) {
			Shader* reflectionShader;
			if (geometry->PBR()) { reflectionShader = ResourceManager::GetInstance()->ReflectionProbeBakingShaderPBR(); }
			else { reflectionShader = ResourceManager::GetInstance()->ReflectionProbeBakingShader(); }
			reflectionShader->Use();

			LightManager::GetInstance()->SetShaderUniforms(reflectionShader, nullptr);

			reflectionShader->setMat4("projection", currentProjection);
			reflectionShader->setVec3("viewPos", currentViewPos);
			reflectionShader->setMat4("view", currentView);

			// setup shader uniforms
			glm::mat4 model = transform->GetWorldModelMatrix();
			reflectionShader->setMat4("model", model);
			reflectionShader->setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
			reflectionShader->setBool("instanced", geometry->Instanced());
			if (geometry->Instanced()) { geometry->BufferInstanceTransforms(); }
			reflectionShader->setVec2("textureScale", geometry->GetTextureScale());
			reflectionShader->setBool("hasBones", false);
			reflectionShader->setBool("OpaqueRenderPass", true);

			// Bones
			if (geometry->GetModel()->HasBones()) {
				if (geometry->GetOwner()->ContainsComponents(COMPONENT_ANIMATOR)) {
					reflectionShader->setBool("hasBones", true);
					std::vector<glm::mat4> transforms = transform->GetOwner()->GetAnimator()->GetFinalBonesMatrices();
					for (int i = 0; i < transforms.size(); i++) {
						reflectionShader->setMat4("boneTransforms[" + std::to_string(i) + "]", transforms[i]);
					}
				}
			}

			// Apply face culling
			if (geometry->Cull_Face()) {
				glEnable(GL_CULL_FACE);
			}
			else {
				glDisable(GL_CULL_FACE);
			}

			if (geometry->Cull_Type() == GL_BACK) {
				///glCullFace(GL_FRONT);
				glCullFace(GL_BACK);
			}
			else if (geometry->Cull_Type() == GL_FRONT) {
				//glCullFace(GL_BACK);
				glCullFace(GL_FRONT);
			}

			// Draw geometry
			geometry->GetModel()->Draw(*reflectionShader, geometry->NumInstances(), geometry->GetInstanceVAOs());

			if (geometry->GetModel()->ContainsTransparentMeshes()) {
				float distanceToCamera = glm::length(activeCamera->GetPosition() - transform->GetWorldPosition());

				if (transparentGeometry.find(distanceToCamera) != transparentGeometry.end()) {
					// Distance already exists, increment slightly
					distanceToCamera += 0.00001f;
				}
				transparentGeometry[distanceToCamera] = geometry;
			}
		}
	}

	void SystemReflectionBaking::DrawTransparentGeometry()
	{
		// Geometry is already sorted in ascending order
		for (std::map<float, ComponentGeometry*>::reverse_iterator it = transparentGeometry.rbegin(); it != transparentGeometry.rend(); ++it) {
			ComponentGeometry* geometry = it->second;

			if (geometry->IsIncludedInReflectionProbes()) {
				Shader* reflectionShader;
				if (geometry->PBR()) { reflectionShader = ResourceManager::GetInstance()->ReflectionProbeBakingShaderPBR(); }
				else { reflectionShader = ResourceManager::GetInstance()->ReflectionProbeBakingShader(); }
				reflectionShader->Use();

				// setup shader uniforms
				glm::mat4 model = geometry->GetOwner()->GetTransformComponent()->GetWorldModelMatrix();
				reflectionShader->setMat4("model", model);
				reflectionShader->setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
				reflectionShader->setBool("instanced", geometry->Instanced());
				if (geometry->Instanced()) { geometry->BufferInstanceTransforms(); }
				reflectionShader->setVec2("textureScale", geometry->GetTextureScale());
				reflectionShader->setBool("hasBones", false);
				reflectionShader->setBool("OpaqueRenderPass", false);

				// Bones
				if (geometry->GetModel()->HasBones()) {
					if (geometry->GetOwner()->ContainsComponents(COMPONENT_ANIMATOR)) {
						reflectionShader->setBool("hasBones", true);
						std::vector<glm::mat4> transforms = geometry->GetOwner()->GetAnimator()->GetFinalBonesMatrices();
						for (int i = 0; i < transforms.size(); i++) {
							reflectionShader->setMat4("boneTransforms[" + std::to_string(i) + "]", transforms[i]);
						}
					}
				}

				// Apply face culling
				if (geometry->Cull_Face()) {
					glEnable(GL_CULL_FACE);
				}
				else {
					glDisable(GL_CULL_FACE);
				}

				if (geometry->Cull_Type() == GL_BACK) {
					///glCullFace(GL_FRONT);
					glCullFace(GL_BACK);
				}
				else if (geometry->Cull_Type() == GL_FRONT) {
					//glCullFace(GL_BACK);
					glCullFace(GL_FRONT);
				}

				geometry->GetModel()->DrawTransparentMeshes(*reflectionShader, geometry->NumInstances(), geometry->GetInstanceVAOs());
			}
		}

		transparentGeometry.clear();
	}
}