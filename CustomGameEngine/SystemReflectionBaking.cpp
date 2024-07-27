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

	void SystemReflectionBaking::Run(const std::vector<Entity*>& entityList)
	{
		std::cout << "SYSTEMREFLECTIONBAKING::Baking reflection probes" << std::endl;
		RenderManager* renderManager = RenderManager::GetInstance();
		unsigned int reflectionFBO = renderManager->GetCubemapFBO();
		unsigned int depthBuffer = renderManager->GetCubemapDepthBuffer();
		Shader* reflectionShader = ResourceManager::GetInstance()->ReflectionProbeBakingShader();

		reflectionShader->Use();
		LightManager::GetInstance()->SetShaderUniforms(reflectionShader, nullptr);

		glBindFramebuffer(GL_FRAMEBUFFER, reflectionFBO);
		
		std::vector<glm::mat4> cubemapTransforms;
		unsigned int width, height;
		std::vector<ReflectionProbe*> probes = renderManager->GetBakedData().GetReflectionProbes();
		int numProbes = probes.size();

		for (int i = 0; i < numProbes; i++) {
			ReflectionProbe* probe = probes[i];
			std::cout << "        Baking probe " << i + 1 << " / " << numProbes << std::endl;

			// Capture scene
			width = probe->GetFaceWidth();
			height = probe->GetFaceHeight();

			glViewport(0, 0, width, height);
			glBindFramebuffer(GL_FRAMEBUFFER, reflectionFBO);

			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, probe->GetProbeEnvMap().cubemapID, 0);

			// Resize depth texture buffer
			glBindTexture(GL_TEXTURE_CUBE_MAP, depthBuffer);
			for (unsigned int j = 0; j < 6; j++) {
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
			}
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthBuffer, 0);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// error check
			int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if (status != GL_FRAMEBUFFER_COMPLETE) {
				std::cout << "ERROR::RENDERMANAGER::Cubemap FBO incomplete, status: 0x\%x\n" << status << std::endl;
			}

			reflectionShader->Use();

			glm::vec3 position = probe->GetWorldPosition();
			glm::mat4 projection = glm::perspective(glm::radians(90.0f), (float)width / (float)height, probe->GetNearClip(), probe->GetFarClip());

			cubemapTransforms.push_back(projection * glm::lookAt(position, position + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
			cubemapTransforms.push_back(projection * glm::lookAt(position, position + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
			cubemapTransforms.push_back(projection * glm::lookAt(position, position + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
			cubemapTransforms.push_back(projection * glm::lookAt(position, position + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
			cubemapTransforms.push_back(projection * glm::lookAt(position, position + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
			cubemapTransforms.push_back(projection * glm::lookAt(position, position + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

			for (unsigned int i = 0; i < 6; ++i) {
				reflectionShader->setMat4("shadowMatrices[" + std::to_string(i) + "]", cubemapTransforms[i]);
			}

			reflectionShader->setVec3("viewPos", position);

			for (Entity* entity : entityList) {
				OnAction(entity);
			}

			cubemapTransforms.clear();

			// Process capture
			glDisable(GL_CULL_FACE);
			unsigned int irradianceMap;
			unsigned int prefilterMap;
			unsigned int brdf_LUT;

			ConvoluteEnvironmentMap(probe);
			PrefilterMap(probe);


			//irradianceMap = renderManager->CreateIrradianceMap();
			//renderManager->ConvoluteEnvironmentMap(probe->GetProbeEnvMap().cubemapID, irradianceMap);
			//prefilterMap = renderManager->CreatePrefilterMap(probe->GetProbeEnvMap().cubemapID);
			brdf_LUT = renderManager->CreateBRDF();

			//probe->GetProbeEnvMap().irradianceID = irradianceMap;
			//probe->GetProbeEnvMap().brdf_lutID = brdf_LUT;
			//probe->GetProbeEnvMap().prefilterID = prefilterMap;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, renderManager->ScreenWidth(), renderManager->ScreenHeight());
	}

	void SystemReflectionBaking::ConvoluteEnvironmentMap(ReflectionProbe* probe)
	{
		RenderManager* renderManager = RenderManager::GetInstance();
		ResourceManager* resourceManager = ResourceManager::GetInstance();

		unsigned int cubeCaptureFBO = *renderManager->GetHDRCubeCaptureFBO();
		unsigned int cubeCaptureRBO = *renderManager->GetHDRCubeCaptureRBO();

		unsigned int envCubemap = probe->GetProbeEnvMap().cubemapID;
		unsigned int irradianceMap = probe->GetProbeEnvMap().irradianceID;

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
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			resourceManager->DefaultCube().DrawWithNoMaterial();
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void SystemReflectionBaking::PrefilterMap(ReflectionProbe* probe)
	{
		RenderManager* renderManager = RenderManager::GetInstance();
		ResourceManager* resourceManager = ResourceManager::GetInstance();

		unsigned int envCubemap = probe->GetProbeEnvMap().cubemapID;
		unsigned int prefilterMap = probe->GetProbeEnvMap().prefilterID;

		unsigned int cubeCaptureFBO = *renderManager->GetHDRCubeCaptureFBO();
		unsigned int cubeCaptureRBO = *renderManager->GetHDRCubeCaptureRBO();

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
		Shader* prefilterShader = ResourceManager::GetInstance()->CreatePrefilterShader();
		prefilterShader->Use();
		prefilterShader->setInt("environmentMap", 0);
		prefilterShader->setMat4("projection", captureProjection);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		glBindFramebuffer(GL_FRAMEBUFFER, cubeCaptureFBO);
		unsigned int maxMipLevels = 5;
		for (unsigned int mip = 0; mip < maxMipLevels; mip++) {
			// resize framebuffer
			unsigned int mipWidth = (128 * 2) * std::pow(0.5, mip);
			unsigned int mipHeight = (128 * 2) * std::pow(0.5, mip);

			glBindRenderbuffer(GL_RENDERBUFFER, cubeCaptureRBO);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);

			glViewport(0, 0, mipWidth, mipHeight);

			float roughness = (float)mip / (float)(maxMipLevels - 1);
			prefilterShader->setFloat("roughness", roughness);
			for (unsigned int i = 0; i < 6; i++) {
				prefilterShader->setMat4("view", captureViews[i]);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				ResourceManager::GetInstance()->DefaultCube().DrawWithNoMaterial();
			}
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void SystemReflectionBaking::OnAction(Entity* entity)
	{
		if ((entity->Mask() & MASK) == MASK) {
			std::vector<Component*> components = entity->Components();

			ComponentTransform* transform = nullptr;
			for (Component* c : components) {
				transform = dynamic_cast<ComponentTransform*>(c);
				if (transform != nullptr) {
					break;
				}
			}

			ComponentGeometry* geometry = nullptr;
			for (Component* c : components) {
				geometry = dynamic_cast<ComponentGeometry*>(c);
				if (geometry != nullptr) {
					break;
				}
			}

			Draw(transform, geometry);
		}
	}

	void SystemReflectionBaking::AfterAction()
	{

	}

	void SystemReflectionBaking::Draw(ComponentTransform* transform, ComponentGeometry* geometry)
	{
		if (geometry->IsIncludedInReflectionProbes()) {
			Shader* reflectionShader = ResourceManager::GetInstance()->ReflectionProbeBakingShader();
			reflectionShader->Use();

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
			geometry->GetModel()->Draw(*reflectionShader, geometry->NumInstances());
		}
	}
}