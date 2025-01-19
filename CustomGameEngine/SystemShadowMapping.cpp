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

	void SystemShadowMapping::Run(const std::vector<Entity*>& entityList)
	{
		SCOPE_TIMER("SystemShadowMapping::Run");
		System::Run(entityList);
	}

	void SystemShadowMapping::OnAction(Entity* entity)
	{
		if ((entity->Mask() & MASK) == MASK) {
			ComponentTransform* transform = entity->GetTransformComponent();
			ComponentGeometry* geometry = entity->GetGeometryComponent();

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
			//depthShader->setBool("instanced", geometry->Instanced());
			//if (geometry->Instanced()) { geometry->BufferInstanceTransforms(); }
			depthShader->setVec2("textureScale", geometry->GetTextureScale());
			depthShader->setBool("hasBones", false);

			// Bones
			//if (geometry->GetModel()->HasBones()) {
			//	if (geometry->GetOwner()->ContainsComponents(COMPONENT_ANIMATOR)) {
			//		depthShader->setBool("hasBones", true);
			//		//std::vector<glm::mat4> transforms = transform->GetOwner()->GetAnimator()->GetFinalBonesMatrices();
			//		//for (int i = 0; i < transforms.size(); i++) {
			//		//	depthShader->setMat4("boneTransforms[" + std::to_string(i) + "]", transforms[i]);
			//		//}
			//	}
			//}

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

			//geometry->GetModel()->Draw(*depthShader, geometry->NumInstances(), geometry->GetInstanceVAOs());
			geometry->GetModel()->Draw(*depthShader, 0, {});
		}
	}
}