#include "SystemAnimatedGeometryAABBGeneration.h"
namespace Engine {
	SystemAnimatedGeometryAABBGeneration::SystemAnimatedGeometryAABBGeneration()
	{
		minMaxVerticesShader = ResourceManager::GetInstance()->LoadComputeShader("Shaders/Compute/verticesMinMax.comp");
		//vertexInput = minMaxVerticesShader->AddNewSSBO(0);
		minMaxOutput = minMaxVerticesShader->AddNewSSBO(1);
	}

	SystemAnimatedGeometryAABBGeneration::~SystemAnimatedGeometryAABBGeneration()
	{

	}

	void SystemAnimatedGeometryAABBGeneration::Run(const std::vector<Entity*>& entityList)
	{
		SCOPE_TIMER("SystemAnimatedGeometryAABBGeneration::Run");
		System::Run(entityList);
	}

	void SystemAnimatedGeometryAABBGeneration::OnAction(Entity* entity)
	{
		SCOPE_TIMER("SystemAnimatedGeometryAABBGeneration::OnAction");
		if ((entity->Mask() & MASK) == MASK) {

			ComponentTransform* transform = entity->GetTransformComponent();
			ComponentGeometry* geometry = entity->GetGeometryComponent();
			ComponentAnimator* animator = entity->GetAnimator();

			if (transform != nullptr && geometry != nullptr && animator != nullptr) {
				GPUComputeAABB(transform, geometry, animator);
			}
		}
	}

	void SystemAnimatedGeometryAABBGeneration::AfterAction()
	{

	}

	void SystemAnimatedGeometryAABBGeneration::GPUComputeAABB(ComponentTransform* transform, ComponentGeometry* geometry, ComponentAnimator* animator)
	{
		SCOPE_TIMER("SystemAnimatedGeometryAABBGeneration::GPUComputeAABB");

		minMaxVerticesShader->Use();

		// Set uniforms
		glm::mat4 model = transform->GetWorldModelMatrix();
		minMaxVerticesShader->setMat4("model", model);

		{
			SCOPE_TIMER("SystemAnimatedGeometryAABBGeneration::GPUComputeAABB::Set bone transforms");
			const int MAX_BONES = 126;
			// Bones
			if (geometry->GetModel()->HasBones()) {
				minMaxVerticesShader->setBool("hasBones", true);
				std::vector<glm::mat4> transforms = transform->GetOwner()->GetAnimator()->GetFinalBonesMatrices();
				for (int i = 0; i < transforms.size() && i < MAX_BONES; i++) {
					minMaxVerticesShader->setMat4("boneTransforms[" + std::to_string(i) + "]", transforms[i]);
				}
			}
		}

		const std::vector<Mesh*>& meshes = geometry->GetModel()->meshes;
		for (Mesh* m : meshes) {
			const unsigned int SSBO = m->GetMeshData().GetSSBO();
			const unsigned int numVertices = m->GetMeshData().GetVertices().size();

			// Initialise output buffer
			minMaxOutput->BufferData(nullptr, 2 * sizeof(glm::ivec4), GL_DYNAMIC_READ);

			// Bind vertex position, boneIDs, boneWeights mesh buffer
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, SSBO);

			// Dispatch compute
			minMaxVerticesShader->DispatchCompute(numVertices / 64, 1, 1, GL_ALL_BARRIER_BITS);

			// Retrieve output
			const float FLOAT_OFFSET = 1000.0f;

			// Read back min/max
			glm::ivec4 min, max;
			minMaxOutput->ReadBufferSubData(&min, sizeof(glm::ivec4), 0);
			minMaxOutput->ReadBufferSubData(&max, sizeof(glm::ivec4), sizeof(glm::ivec4));

			glm::vec3 floatMin = glm::vec3(
				glm::intBitsToFloat(min.x) - FLOAT_OFFSET,
				glm::intBitsToFloat(min.y) - FLOAT_OFFSET,
				glm::intBitsToFloat(min.z) - FLOAT_OFFSET
			);

			glm::vec3 floatMax = glm::vec3(
				glm::intBitsToFloat(max.x) - FLOAT_OFFSET,
				glm::intBitsToFloat(max.y) - FLOAT_OFFSET,
				glm::intBitsToFloat(max.z) - FLOAT_OFFSET
			);

			// Update geometry AABB
			//std::cout << "Min: x_" << floatMin.x << "/y_" << floatMin.y << "/z_" << floatMin.z << " ||| Max: x_" << floatMax.x << "/y_" << floatMax.y << "/z" << floatMax.z << std::endl;
			m->GetGeometryAABB().SetMinMax(floatMin, floatMax);
		}
	}
}