#include "SystemRender.h"
#include "LightManager.h"
#include "ResourceManager.h"
namespace Engine {
	std::map<float, Mesh*> SystemRender::transparentMeshes = std::map<float, Mesh*>();
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

	void SystemRender::Run(const std::vector<Entity*>& entityList)
	{
		SCOPE_TIMER("SystemRender::Run");
		System::Run(entityList);
	}

	void SystemRender::OnAction(Entity* entity)
	{
		SCOPE_TIMER("SystemRender::OnAction");
		if ((entity->Mask() & MASK) == MASK) {
			ComponentTransform* transform = entity->GetTransformComponent();
			ComponentGeometry* geometry = entity->GetGeometryComponent();

			Draw(transform, geometry);
		}
	}

	void SystemRender::AfterAction()
	{
		SCOPE_TIMER("SystemRender::AfterAction");
		shadersUsedThisFrame.clear();
	}

	void SystemRender::Draw(ComponentTransform* transform, ComponentGeometry* geometry)
	{
		SCOPE_TIMER("SystemRender::Draw");
		Shader* shader = geometry->GetShader();

		{
			SCOPE_TIMER("SystemRender::Draw::Check shader used this frame");
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
		}

		{
			SCOPE_TIMER("SystemRender::Draw::Set base uniforms");
			shader->Use();

			glm::mat4 model = transform->GetWorldModelMatrix();
			shader->setMat4("model", model);
			shader->setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
			shader->setBool("instanced", geometry->Instanced());
			if (geometry->Instanced()) { geometry->BufferInstanceTransforms(); }
			shader->setBool("hasBones", false);
			shader->setBool("OpaqueRenderPass", true);
		}

		{
			SCOPE_TIMER("SystemRender::Draw::Set bone transforms");
			// Bones
			if (geometry->GetModel()->HasBones()) {
				if (geometry->GetOwner()->ContainsComponents(COMPONENT_ANIMATOR)) {
					shader->setBool("hasBones", true);
					//std::vector<glm::mat4> transforms = transform->GetOwner()->GetAnimator()->GetFinalBonesMatrices();
					//for (int i = 0; i < transforms.size(); i++) {
						//shader->setMat4("boneTransforms[" + std::to_string(i) + "]", transforms[i]);
					//}
				}
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

		geometry->GetModel()->Draw(*geometry->GetShader(), geometry->NumInstances(), geometry->GetInstanceVAOs());

		if (geometry->GetModel()->ContainsTransparentMeshes()) {
			float distanceToCamera = glm::length(activeCamera->GetPosition() - transform->GetWorldPosition());

			int iterations = 0;
			while (transparentGeometry.find(distanceToCamera) != transparentGeometry.end()) {
				// Distance already exists, increment slightly
				distanceToCamera += 0.001f;
				iterations++;
				if (iterations > 100) {
					std::cout << "aahhhhh" << std::endl;
				}
			}
			transparentGeometry[distanceToCamera] = geometry;
		}
	}

	void SystemRender::DrawTransparentGeometry(bool useDefaultForwardShader)
	{
		SCOPE_TIMER("SystemRender::DrawTransparentGeometry");
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
			shader->setVec2("textureScale", geometry->GetTextureScale());
			shader->setBool("hasBones", false);
			shader->setBool("OpaqueRenderPass", false);

			// Bones
			if (geometry->GetModel()->HasBones()) {
				if (geometry->GetOwner()->ContainsComponents(COMPONENT_ANIMATOR)) {
					shader->setBool("hasBones", true);
					//std::vector<glm::mat4> transforms = transform->GetOwner()->GetAnimator()->GetFinalBonesMatrices();
					//for (int i = 0; i < transforms.size(); i++) {
					//	shader->setMat4("boneTransforms[" + std::to_string(i) + "]", transforms[i]);
					//}
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

			geometry->GetModel()->DrawTransparentMeshes(*shader, geometry->NumInstances(), geometry->GetInstanceVAOs());
		}

		transparentGeometry.clear();
	}

	void SystemRender::RenderMeshes(const std::map<float, Mesh*>& meshesAndDistances, const bool transparencyPass, bool useDefaultForwardShader)
	{
		std::map<float, Mesh*>::const_iterator meshesAndDistancesIt = meshesAndDistances.begin();

		while (meshesAndDistancesIt != meshesAndDistances.end()) {
			Mesh* m = meshesAndDistancesIt->second;
			RenderMesh(m, transparencyPass, useDefaultForwardShader);
			if (!transparencyPass && m->GetMaterial()->GetIsTransparent()) {
				transparentMeshes[meshesAndDistancesIt->first] = m;
			}

			meshesAndDistancesIt++;
		}

		if (transparencyPass) { transparentMeshes.clear(); }
	}

	void SystemRender::RenderMeshes(const std::vector<Mesh*>& meshList, const bool transparencyPass, bool useDefaultForwardShader)
	{
		for (Mesh* m : meshList) {
			RenderMesh(m, transparencyPass, useDefaultForwardShader);
			if (!transparencyPass && m->GetMaterial()->GetIsTransparent()) {
				AddMeshToTransparentMeshes(m->GetOwner()->GetOwner()->GetOwner()->GetTransformComponent(), m);
			}
		}

		if (transparencyPass) { transparentMeshes.clear(); }
	}

	void SystemRender::RenderMesh(Mesh* mesh, const bool transparencyPass, bool useDefaultForwardShader)
	{
		SCOPE_TIMER("SystemRender::RenderMesh");
		ComponentTransform* transform = mesh->GetOwner()->GetOwner()->GetOwner()->GetTransformComponent();
		ComponentGeometry* geometry = mesh->GetOwner()->GetOwner();

		Shader* shader;
		if (useDefaultForwardShader) { if (geometry->PBR()) { shader = ResourceManager::GetInstance()->DefaultLitPBR(); } else { shader = ResourceManager::GetInstance()->DefaultLitShader(); } }
		else { shader = geometry->GetShader(); }

		{
			SCOPE_TIMER("SystemRender::RenderMesh::Check shader used this frame");
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
		}

		{
			SCOPE_TIMER("SystemRender::RenderMesh::Set base uniforms");
			shader->Use();

			glm::mat4 model = transform->GetWorldModelMatrix();
			shader->setMat4("model", model);
			shader->setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
			shader->setBool("instanced", geometry->Instanced());
			if (geometry->Instanced()) { geometry->BufferInstanceTransforms(); }
			shader->setBool("hasBones", false);
			shader->setBool("OpaqueRenderPass", !transparencyPass);
		}

		{
			SCOPE_TIMER("SystemRender::RenderMesh::Set bone transforms");
			// Bones
			if (geometry->GetModel()->HasBones()) {
				if (geometry->GetOwner()->ContainsComponents(COMPONENT_ANIMATOR)) {
					shader->setBool("hasBones", true);
					//std::vector<glm::mat4> transforms = transform->GetOwner()->GetAnimator()->GetFinalBonesMatrices();
					//for (int i = 0; i < transforms.size(); i++) {
					//	shader->setMat4("boneTransforms[" + std::to_string(i) + "]", transforms[i]);
					//}
				}
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

		unsigned int numInstances = geometry->NumInstances();

		if (numInstances > 0) {
			mesh->Draw(*shader, geometry->PBR(), numInstances, geometry->GetInstanceVAOs()[mesh->GetLocalMeshID()]);
		}
		else {
			mesh->Draw(*shader, geometry->PBR());
		}
	}

	void SystemRender::AddMeshToTransparentMeshes(ComponentTransform* transform, Mesh* mesh)
	{
		SCOPE_TIMER("SystemRender::AddMeshToTransparentMeshes");
		float distanceToCamera = glm::length(activeCamera->GetPosition() - transform->GetWorldPosition());

		int iterations = 0;
		while (transparentMeshes.find(distanceToCamera) != transparentMeshes.end()) {
			distanceToCamera += 0.001f;
			iterations++;
			if (iterations > 100) {
				std::cout << "ahhhhh" << std::endl;
			}
		}
		transparentMeshes[distanceToCamera] = mesh;
	}
}