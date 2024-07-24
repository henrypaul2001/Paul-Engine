#include "SystemReflectionBaking.h"
namespace Engine {
	SystemReflectionBaking::SystemReflectionBaking()
	{

	}

	SystemReflectionBaking::~SystemReflectionBaking()
	{

	}

	void SystemReflectionBaking::Run(const std::vector<Entity*>& entityList)
	{

		for (Entity* entity : entityList) {
			OnAction(entity);
		}
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
		// if geometry included in reflection probes
			// get reflection probe shader
			Shader* reflectionShader = new Shader("null", "null");

			// setup shader uniforms
			reflectionShader->setMat4("model", transform->GetWorldModelMatrix());
			reflectionShader->setBool("instanced", geometry->Instanced());
			if (geometry->Instanced()) { geometry->BufferInstanceTransforms(); }
			reflectionShader->setVec2("textureScale", geometry->GetTextureScale());
			reflectionShader->setBool("hasBones", false);

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