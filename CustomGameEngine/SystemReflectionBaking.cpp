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

			width = probe->GetFaceWidth();
			height = probe->GetFaceHeight();

			glViewport(0, 0, width, height);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, probe->GetProbeEnvMap().cubemapID, 0);

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
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, renderManager->ScreenWidth(), renderManager->ScreenHeight());
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