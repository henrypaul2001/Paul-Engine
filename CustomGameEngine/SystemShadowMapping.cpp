#include "SystemShadowMapping.h"
#include "ResourceManager.h"
namespace Engine {
	SystemShadowMapping::SystemShadowMapping()
	{
		type = MAP_2D;
	}

	SystemShadowMapping::~SystemShadowMapping()
	{

	}

	void SystemShadowMapping::OnAction(Entity* entity)
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

	void SystemShadowMapping::AfterAction()
	{

	}

	void SystemShadowMapping::Draw(ComponentTransform* transform, ComponentGeometry* geometry)
	{
		if (geometry->CastShadows()) {
			Shader* depthShader = nullptr;
			if (type == MAP_2D) {
				depthShader = ResourceManager::GetInstance()->ShadowMapShader();
			}
			else {
				depthShader = ResourceManager::GetInstance()->CubeShadowMapShader();
			}

			depthShader->setMat4("model", transform->GetWorldModelMatrix());
			depthShader->setBool("instanced", geometry->Instanced());
			if (geometry->Instanced()) { geometry->BufferInstanceTransforms(); }

			// Bones
			if (geometry->GetModel()->HasBones()) {
				const AnimationSkeleton& skeleton = geometry->GetModel()->GetAnimationSkeleton();
				for (int i = 0; i < skeleton.finalBoneMatrices.size(); i++) {
					depthShader->setMat4("boneTransforms[" + std::to_string(i) + "]", skeleton.finalBoneMatrices[i]);
				}
			}

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

			geometry->GetModel()->Draw(*depthShader, geometry->NumInstances());
		}
	}
}