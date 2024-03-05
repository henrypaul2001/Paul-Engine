#include "SystemRender.h"
#include "LightManager.h"

namespace Engine {
	SystemRender::SystemRender()
	{
		//camera = nullptr;
		shadersUsedThisFrame = std::vector<Shader*>();
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
					LightManager::GetInstance()->SetShaderUniforms(shader);
					break;
				}
			}
		}
		else {
			shadersUsedThisFrame.push_back(shader);
			shader->Use();
			LightManager::GetInstance()->SetShaderUniforms(shader);
		}

		shader->Use();

		//LightManager::GetInstance()->SetShaderUniforms(shader);

		glm::mat4 model = transform->GetWorldModelMatrix();
		shader->setMat4("model", model);
		shader->setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
		shader->setFloat("textureScale", geometry->GetTextureScale());
		//shader->setFloat("material.SHININESS", 13.72f);


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
			glCullFace(GL_FRONT);
		}

		geometry->GetModel()->Draw(*geometry->GetShader());
	}
}