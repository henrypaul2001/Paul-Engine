#include "SystemRender.h"
#include "LightManager.h"
#include "ResourceManager.h"
namespace Engine {
	SystemRender::SystemRender()
	{
		//camera = nullptr;
		shadersUsedThisFrame = std::vector<Shader*>();
		postProcess = NONE;
		PostProcessKernel[0] = 0.0;
		PostProcessKernel[1] = 0.0;
		PostProcessKernel[2] = 0.0;
		PostProcessKernel[3] = 0.0;
		PostProcessKernel[4] = 1.0;
		PostProcessKernel[5] = 0.0;
		PostProcessKernel[6] = 0.0;
		PostProcessKernel[7] = 0.0;
		PostProcessKernel[8] = 0.0;
	}

	SystemRender::~SystemRender()
	{

	}

	void SystemRender::OnAction(Entity* entity)
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

	void SystemRender::AfterAction()
	{
		shadersUsedThisFrame.clear();
	}

	void SystemRender::Draw(ComponentTransform* transform, ComponentGeometry* geometry)
	{
		Shader* shader = geometry->GetShader();
		if (shadersUsedThisFrame.size() > 0) {
			for (Shader* s : shadersUsedThisFrame) {
				if (s->GetID() == shader->GetID()) {
					// lighting uniforms already set
				}
				else {
					// add shader to list and set lighting uniforms
					shadersUsedThisFrame.push_back(s);
					shader->Use();
					LightManager::GetInstance()->SetShaderUniforms(shader, activeCamera);
					break;
				}
			}
		}
		else {
			shadersUsedThisFrame.push_back(shader);
			shader->Use();
			LightManager::GetInstance()->SetShaderUniforms(shader, activeCamera);
		}

		shader->Use();

		glm::mat4 model = transform->GetWorldModelMatrix();
		shader->setMat4("model", model);
		shader->setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
		shader->setBool("instanced", geometry->Instanced());
		if (geometry->Instanced()) { geometry->BufferInstanceTransforms(); }
		shader->setFloat("textureScale", geometry->GetTextureScale());
		shader->setBool("hasBones", false);

		// Bones
		if (geometry->GetModel()->HasBones()) {
			shader->setBool("hasBones", true);
			std::vector<glm::mat4> transforms = transform->GetOwner()->GetAnimator()->GetFinalBonesMatrices();
			for (int i = 0; i < transforms.size(); i++) {
				shader->setMat4("boneTransforms[" + std::to_string(i) + "]", transforms[i]);
			}
		}

		if (geometry->Cull_Face()) {
			glEnable(GL_CULL_FACE);
		}
		else {
			glDisable(GL_CULL_FACE);
		}

		if (geometry->Cull_Type() == GL_BACK) {
			glCullFace(GL_BACK);
		}
		else if (geometry->Cull_Type() == GL_FRONT) {
			glCullFace(GL_FRONT);
		}
		else {
			glCullFace(GL_BACK);
		}

		geometry->GetModel()->Draw(*geometry->GetShader(), geometry->NumInstances());

		if (geometry->GetModel()->ContainsTransparentMeshes()) {
			float distanceToCamera = glm::length(activeCamera->Position - transform->GetWorldPosition());

			if (transparentGeometry.find(distanceToCamera) != transparentGeometry.end()) {
				// Distance already exists, increment slightly
				distanceToCamera += 0.00001f;
			}
			transparentGeometry[distanceToCamera] = geometry;
		}
	}

	void SystemRender::DrawTransparentGeometry(bool useDefaultForwardShader)
	{
		// Geometry is already sorted in ascending order
		for (std::map<float, ComponentGeometry*>::reverse_iterator it = transparentGeometry.rbegin(); it != transparentGeometry.rend(); ++it) {
			ComponentGeometry* geometry = it->second;

			Shader* shader;
			if (useDefaultForwardShader) {
				if (geometry->PBR()) {
					shader = ResourceManager::GetInstance()->DefaultLitPBR();
				}
				else {
					shader = ResourceManager::GetInstance()->DefaultLitShader();
				}
			}
			else {
				shader = geometry->GetShader();
			}

			shader->Use();

			ComponentTransform* transform = dynamic_cast<ComponentTransform*>(geometry->GetOwner()->GetComponent(COMPONENT_TRANSFORM));
			glm::mat4 model = transform->GetWorldModelMatrix();
			shader->setMat4("model", model);
			shader->setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
			shader->setBool("instanced", geometry->Instanced());
			if (geometry->Instanced()) { geometry->BufferInstanceTransforms(); }
			shader->setFloat("textureScale", geometry->GetTextureScale());
			shader->setBool("hasBones", false);

			// Bones
			if (geometry->GetModel()->HasBones()) {
				shader->setBool("hasBones", true);
				std::vector<glm::mat4> transforms = transform->GetOwner()->GetAnimator()->GetFinalBonesMatrices();
				for (int i = 0; i < transforms.size(); i++) {
					shader->setMat4("boneTransforms[" + std::to_string(i) + "]", transforms[i]);
				}
			}

			if (geometry->Cull_Face()) {
				glEnable(GL_CULL_FACE);
			}
			else {
				glDisable(GL_CULL_FACE);
			}

			if (geometry->Cull_Type() == GL_BACK) {
				glCullFace(GL_BACK);
			}
			else if (geometry->Cull_Type() == GL_FRONT) {
				glCullFace(GL_FRONT);
			}
			else {
				glCullFace(GL_BACK);
			}

			geometry->GetModel()->DrawTransparentMeshes(*shader, geometry->NumInstances());
		}

		transparentGeometry.clear();
	}
}