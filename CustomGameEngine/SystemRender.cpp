#include "SystemRender.h"
#include "LightManager.h"
#include "ResourceManager.h"
namespace Engine {
	std::map<float, std::pair<Mesh*, unsigned int>> SystemRender::transparentMeshes = std::map<float, std::pair<Mesh*, unsigned int>>();
	SystemRender::SystemRender(EntityManagerNew* ecs) : ecs(ecs)
	{
		//camera = nullptr;
		shadersUsedThisFrame = std::unordered_map<unsigned int, Shader*>();
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

	SystemRender::~SystemRender() {}

	void SystemRender::AfterAction()
	{
		SCOPE_TIMER("SystemRender::AfterAction");
		shadersUsedThisFrame.clear();
	}

	void SystemRender::RenderMeshes(const std::map<float, std::pair<Mesh*, unsigned int>>& meshesAndDistances, const bool transparencyPass, bool useDefaultForwardShader)
	{
		std::map<float, std::pair<Mesh*, unsigned int>>::const_iterator meshesAndDistancesIt = meshesAndDistances.begin();

		while (meshesAndDistancesIt != meshesAndDistances.end()) {
			Mesh* m = meshesAndDistancesIt->second.first;
			RenderMesh(meshesAndDistancesIt->second.second, m, transparencyPass, useDefaultForwardShader);
			if (!transparencyPass && m->GetMaterial()->GetIsTransparent()) {
				transparentMeshes[meshesAndDistancesIt->first] = meshesAndDistancesIt->second;
			}

			meshesAndDistancesIt++;
		}

		if (transparencyPass) { transparentMeshes.clear(); }
	}

	void SystemRender::RenderMesh(const unsigned int entityID, Mesh* mesh, const bool transparencyPass, bool useDefaultForwardShader)
	{
		SCOPE_TIMER("SystemRender::RenderMesh");
		ResourceManager* resources = ResourceManager::GetInstance();
		LightManager* lightManager = LightManager::GetInstance();

		const ComponentTransform* transform = ecs->GetComponent<ComponentTransform>(entityID);
		ComponentGeometry* geometry = ecs->GetComponent<ComponentGeometry>(entityID);

		Shader* shader;
		if (useDefaultForwardShader) { if (geometry->PBR()) { shader = resources->DefaultLitPBR(); } else { shader = resources->DefaultLitShader(); } }
		else { shader = geometry->GetShader(); }

		{
			SCOPE_TIMER("SystemRender::RenderMesh::Check shader used this frame");
			if (shadersUsedThisFrame.find(shader->GetID()) != shadersUsedThisFrame.end()) {
				// add shader to list and set lighting uniforms
				shadersUsedThisFrame[shader->GetID()] = shader;
				shader->Use();
				lightManager->SetShaderUniforms(shader, activeCamera);
			}
		}

		{
			SCOPE_TIMER("SystemRender::RenderMesh::Set base uniforms");
			shader->Use();

			const glm::mat4& model = transform->GetWorldModelMatrix();
			shader->setMat4("model", model);
			shader->setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
			shader->setBool("instanced", false);
			//shader->setBool("instanced", geometry->Instanced());
			//if (geometry->Instanced()) { geometry->BufferInstanceTransforms(); }
			shader->setBool("hasBones", false);
			shader->setBool("OpaqueRenderPass", !transparencyPass);
		}

		{
			SCOPE_TIMER("SystemRender::RenderMesh::Set bone transforms");
			// Bones
			if (geometry->GetModel()->HasBones()) {
				ComponentAnimator* animator = ecs->GetComponent<ComponentAnimator>(entityID);
				if (animator) {
					shader->setBool("hasBones", true);
					const std::vector<glm::mat4>& transforms = animator->GetFinalBonesMatrices();
					for (int i = 0; i < transforms.size(); i++) {
						shader->setMat4("boneTransforms[" + std::to_string(i) + "]", transforms[i]);
					}
				}
			}
		}

		if (geometry->Cull_Face()) { glEnable(GL_CULL_FACE); }
		else { glDisable(GL_CULL_FACE); }

		if (geometry->Cull_Type() == GL_BACK) { glCullFace(GL_BACK); }
		else if (geometry->Cull_Type() == GL_FRONT) { glCullFace(GL_FRONT); }
		else { glCullFace(GL_BACK); }

		unsigned int numInstances = 0;
		//unsigned int numInstances = geometry->NumInstances();

		//if (numInstances > 0) {
		//	mesh->Draw(*shader, geometry->PBR(), numInstances, geometry->GetInstanceVAOs()[mesh->GetLocalMeshID()]);
		//}
		//else {
		//	mesh->Draw(*shader, geometry->PBR());
		//}
		mesh->Draw(*shader, geometry->PBR());
	}
}