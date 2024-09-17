#pragma once
#include "System.h"
#include "ComputeShader.h"
#include "ResourceManager.h"
#include <algorithm>
#include <iterator>
namespace Engine {
	class SystemAnimatedGeometryAABBGeneration : public System
	{
		struct MinMaxGPUReadBack {
			glm::ivec4 min;
			glm::ivec4 max;
		};

		struct MeshEntry {
			Mesh* mesh;
			ComponentTransform* transform;
			ComponentGeometry* geometry;
			ComponentAnimator* animator;
		};

	public:
		SystemAnimatedGeometryAABBGeneration();
		~SystemAnimatedGeometryAABBGeneration();

		SystemTypes Name() override { return SYSTEM_ANIMATED_GEOMETRY_AABB; }
		void Run(const std::vector<Entity*>& entityList) override;
		void OnAction(Entity* entity) override;
		void AfterAction() override;

	private:
		const ComponentTypes MASK = (COMPONENT_TRANSFORM | COMPONENT_GEOMETRY | COMPONENT_ANIMATOR);

		std::vector<MeshEntry> meshList;

		void GPUComputeAABB(MeshEntry meshEntry);

		ComputeShader* minMaxVerticesShader;
		const ShaderStorageBuffer* minMaxOutput;
		glm::ivec4* outputBuffer;
	};
}