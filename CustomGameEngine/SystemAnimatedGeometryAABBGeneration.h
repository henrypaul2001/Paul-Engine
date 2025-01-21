#pragma once
#include "SystemNew.h"
#include "ComputeShader.h"
#include "ResourceManager.h"
#include <algorithm>
#include <iterator>

#include "ComponentTransform.h"
#include "ComponentGeometry.h"
#include "ComponentAnimator.h"

namespace Engine {
	class SystemAnimatedGeometryAABBGeneration : public SystemNew
	{
		struct MinMaxGPUReadBack {
			glm::ivec4 min;
			glm::ivec4 max;
		};

		struct MeshEntry {
			Mesh* mesh;
			ComponentTransform& transform;
			ComponentGeometry& geometry;
			ComponentAnimator& animator;
		};

	public:
		SystemAnimatedGeometryAABBGeneration(EntityManagerNew* ecs) : SystemNew(ecs) {
			minMaxVerticesShader = ResourceManager::GetInstance()->LoadComputeShader("Shaders/Compute/verticesMinMax.comp");
			minMaxOutput = minMaxVerticesShader->AddNewSSBO(1);
		}
		~SystemAnimatedGeometryAABBGeneration() {}

		constexpr const char* SystemName() override { return "SYSTEM_ANIMATED_GEOMETRY_AABB"; }

		void OnAction(const unsigned int entityID, ComponentTransform& transform, ComponentGeometry& geometry, ComponentAnimator& animator);
		void AfterAction();

	private:
		std::vector<MeshEntry> meshList;

		void GPUComputeAABB(const MeshEntry meshEntry);

		ComputeShader* minMaxVerticesShader;
		const ShaderStorageBuffer* minMaxOutput;
		glm::ivec4* outputBuffer;
	};
}