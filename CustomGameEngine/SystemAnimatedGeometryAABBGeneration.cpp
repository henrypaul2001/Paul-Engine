#include "SystemAnimatedGeometryAABBGeneration.h"
namespace Engine {
	void SystemAnimatedGeometryAABBGeneration::OnAction(const unsigned int entityID, ComponentTransform& transform, ComponentGeometry& geometry, ComponentAnimator& animator)
	{
		SCOPE_TIMER("SystemAnimatedGeometryAABBGeneration::OnAction");

		const std::vector<Mesh*>& meshes = geometry.GetModel()->meshes;
		for (Mesh* m : meshes) {
			meshList.push_back({ m, transform, geometry, animator });
		}
	}

	void SystemAnimatedGeometryAABBGeneration::AfterAction() {
		SCOPE_TIMER("SystemAnimatedGeometryAABBGeneration::AfterAction");

		const unsigned int numMeshes = meshList.size();
		minMaxVerticesShader->Use();

		// Initialise output buffer
		minMaxOutput->BufferData(nullptr, numMeshes * (2 * sizeof(glm::ivec4)), GL_DYNAMIC_READ);

		{
			// Run compute shader on each mesh
			SCOPE_TIMER("SystemAnimatedGeometryAABBGeneration::AfterAction::ComputeMeshes");
			for (unsigned int i = 0; i < numMeshes; i++) {
				minMaxVerticesShader->setUInt("meshID", i);
				GPUComputeAABB(meshList[i]);
			}
		}

		// Read back data
		const float FLOAT_OFFSET = 1000.0f;

		const unsigned int maxOutputs = 1000;
		MinMaxGPUReadBack output[maxOutputs];
		if (numMeshes >= maxOutputs) {
			std::cout << "Oops" << std::endl;
		}

		{
			// Read buffer
			SCOPE_TIMER("SystemAnimatedGeometryAABBGeneration::AfterAction::ReadOutputBuffer");
			minMaxOutput->ReadBufferData(&output);
		}

		{
			// Update AABBs
			SCOPE_TIMER("SystemAnimatedGeometryAABBGeneration::AfterAction::UpdateAABBs");
			for (unsigned int i = 0; i < numMeshes && i < maxOutputs; i++) {

				const glm::vec3 floatMin = glm::vec3(
					glm::intBitsToFloat(output[i].min.x) - FLOAT_OFFSET,
					glm::intBitsToFloat(output[i].min.y) - FLOAT_OFFSET,
					glm::intBitsToFloat(output[i].min.z) - FLOAT_OFFSET
				);

				const glm::vec3 floatMax = glm::vec3(
					glm::intBitsToFloat(output[i].max.x) - FLOAT_OFFSET,
					glm::intBitsToFloat(output[i].max.y) - FLOAT_OFFSET,
					glm::intBitsToFloat(output[i].max.z) - FLOAT_OFFSET
				);

				meshList[i].mesh->GetGeometryAABB().SetMinMax(floatMin, floatMax);
			}
		}

		meshList.clear();
	}

	void SystemAnimatedGeometryAABBGeneration::GPUComputeAABB(const MeshEntry meshEntry)
	{
		SCOPE_TIMER("SystemAnimatedGeometryAABBGeneration::GPUComputeAABB");
		ComponentTransform& transform = meshEntry.transform;
		ComponentGeometry& geometry = meshEntry.geometry;
		ComponentAnimator& animator = meshEntry.animator;
		Mesh* m = meshEntry.mesh;

		// Set uniforms
		const glm::mat4 model = transform.GetWorldModelMatrix();
		minMaxVerticesShader->setMat4("model", model);

		{
			SCOPE_TIMER("SystemAnimatedGeometryAABBGeneration::GPUComputeAABB::Set bone transforms");
			const int MAX_BONES = 126;
			// Bones
			if (geometry.GetModel()->HasBones()) {
				minMaxVerticesShader->setBool("hasBones", true);
				const std::vector<glm::mat4>& transforms = animator.GetFinalBonesMatrices();
				for (int i = 0; i < transforms.size() && i < MAX_BONES; i++) {
					minMaxVerticesShader->setMat4("boneTransforms[" + std::to_string(i) + "]", transforms[i]);
				}
			}
		}

		const unsigned int SSBO = m->GetMeshData().GetSSBO();
		const unsigned int numVertices = m->GetMeshData().GetVertices().size();
		{
			SCOPE_TIMER(std::string("SystemAnimatedGeometryAABBGeneration::ProcessMesh::NoVertices = " + std::to_string(numVertices)).c_str());

			// Bind vertex position, boneIDs, boneWeights mesh buffer
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, SSBO);

			// Dispatch compute
			minMaxVerticesShader->DispatchCompute(numVertices / 64, 1, 1, GL_SHADER_STORAGE_BARRIER_BIT); // room for further optimisation. Single compute dispatch with y work groups handling a mesh each
		}
	}
}