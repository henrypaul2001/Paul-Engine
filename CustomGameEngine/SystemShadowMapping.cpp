#include "SystemShadowMapping.h"
#include "ComponentAnimator.h"
#include "ResourceManager.h"
namespace Engine {
	void SystemShadowMapping::OnAction(const unsigned int entityID, ComponentTransform& transform, ComponentGeometry& geometry)
	{
		SCOPE_TIMER("SystemShadowMapping::OnAction()");
		if (geometry.CastShadows()) {
			Shader* depthShader = nullptr;
			if (type == MAP_2D) {
				depthShader = ResourceManager::GetInstance()->ShadowMapShader();
			}
			else {
				depthShader = ResourceManager::GetInstance()->CubeShadowMapShader();
			}

			depthShader->setMat4("model", transform.GetWorldModelMatrix());
			//depthShader->setBool("instanced", geometry->Instanced());
			//if (geometry->Instanced()) { geometry->BufferInstanceTransforms(); }
			depthShader->setVec2("textureScale", geometry.GetTextureScale());
			depthShader->setBool("hasBones", false);

			// Bones
			if (geometry.GetModel()->HasBones()) {
				ComponentAnimator* animator = active_ecs->GetComponent<ComponentAnimator>(entityID);
				if (animator) {
					depthShader->setBool("hasBones", true);
					const std::vector<glm::mat4>& transforms = animator->GetFinalBonesMatrices();
					for (int i = 0; i < transforms.size(); i++) {
						depthShader->setMat4("boneTransforms[" + std::to_string(i) + "]", transforms[i]);
					}
				}
			}

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

			//geometry->GetModel()->Draw(*depthShader, geometry->NumInstances(), geometry->GetInstanceVAOs());
			geometry.GetModel()->Draw(*depthShader, 0, {});
		}
	}
}