#include "SystemAnimatedGeometryAABBGeneration.h"
#include "ComponentTransform.h"
namespace Engine {
	SystemAnimatedGeometryAABBGeneration::SystemAnimatedGeometryAABBGeneration()
	{
		minMaxVerticesShader = ResourceManager::GetInstance()->LoadComputeShader("Shaders/Compute/verticesMinMax.comp");
		minMaxOutput = minMaxVerticesShader->AddNewSSBO(1);
	}

	SystemAnimatedGeometryAABBGeneration::~SystemAnimatedGeometryAABBGeneration()
	{
	}

	void SystemAnimatedGeometryAABBGeneration::Run(const std::vector<Entity*>& entityList)
	{
		SCOPE_TIMER("SystemAnimatedGeometryAABBGeneration::Run");
		System::Run(entityList);

		const unsigned int numMeshes = meshList.size();
		minMaxVerticesShader->Use();

		// Initialise output buffer
		minMaxOutput->BufferData(nullptr, numMeshes * (2 * sizeof(glm::ivec4)), GL_DYNAMIC_READ);

		{
			// Run compute shader on each mesh
			SCOPE_TIMER("SystemAnimatedGeometryAABBGeneration::Run::ComputeMeshes");
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
			SCOPE_TIMER("SystemAnimatedGeometryAABBGeneration::Run::ReadOutputBuffer");
			minMaxOutput->ReadBufferData(&output);
		}

		{
			// Update AABBs
			SCOPE_TIMER("SystemAnimatedGeometryAABBGeneration::Run::UpdateAABBs");
			glm::vec3 floatMin, floatMax;
			for (unsigned int i = 0; i < numMeshes && i < maxOutputs; i++) {

				floatMin = glm::vec3(
					glm::intBitsToFloat(output[i].min.x) - FLOAT_OFFSET,
					glm::intBitsToFloat(output[i].min.y) - FLOAT_OFFSET,
					glm::intBitsToFloat(output[i].min.z) - FLOAT_OFFSET
				);

				floatMax = glm::vec3(
					glm::intBitsToFloat(output[i].max.x) - FLOAT_OFFSET,
					glm::intBitsToFloat(output[i].max.y) - FLOAT_OFFSET,
					glm::intBitsToFloat(output[i].max.z) - FLOAT_OFFSET
				);

				meshList[i].mesh->GetGeometryAABB().SetMinMax(floatMin, floatMax);
			}
		}

		meshList.clear();
	}

	void SystemAnimatedGeometryAABBGeneration::OnAction(Entity* entity)
	{
		SCOPE_TIMER("SystemAnimatedGeometryAABBGeneration::OnAction");
		if ((entity->Mask() & MASK) == MASK) {

			ComponentTransform* transform = entity->GetTransformComponent();
			ComponentGeometry* geometry = entity->GetGeometryComponent();
			ComponentAnimator* animator = entity->GetAnimator();

			if (transform != nullptr && geometry != nullptr && animator != nullptr) {
				const std::vector<Mesh*>& meshes = geometry->GetModel()->meshes;
				for (Mesh* m : meshes) {
					meshList.push_back({ m, transform, geometry, animator });
				}
			}
		}
	}

	void SystemAnimatedGeometryAABBGeneration::AfterAction() {}

	void SystemAnimatedGeometryAABBGeneration::GPUComputeAABB(MeshEntry meshEntry)
	{
		SCOPE_TIMER("SystemAnimatedGeometryAABBGeneration::GPUComputeAABB");
		ComponentTransform* transform = meshEntry.transform;
		ComponentGeometry* geometry = meshEntry.geometry;
		ComponentAnimator* animator = meshEntry.animator;
		Mesh* m = meshEntry.mesh;

		// Set uniforms
		glm::mat4 model = transform->GetWorldModelMatrix();
		minMaxVerticesShader->setMat4("model", model);

		{
			SCOPE_TIMER("SystemAnimatedGeometryAABBGeneration::GPUComputeAABB::Set bone transforms");
			const int MAX_BONES = 126;
			// Bones
			if (geometry->GetModel()->HasBones()) {
				minMaxVerticesShader->setBool("hasBones", true);
				std::vector<glm::mat4> transforms = animator->GetFinalBonesMatrices();
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