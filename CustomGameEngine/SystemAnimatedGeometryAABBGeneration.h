#pragma once
#include "System.h"
#include "ComputeShader.h"
#include "ResourceManager.h"
#include <algorithm>
#include <iterator>
namespace Engine {
	class SystemAnimatedGeometryAABBGeneration : public System
	{
	public:
		SystemAnimatedGeometryAABBGeneration();
		~SystemAnimatedGeometryAABBGeneration();

		SystemTypes Name() override { return SYSTEM_ANIMATED_GEOMETRY_AABB; }
		void Run(const std::vector<Entity*>& entityList) override;
		void OnAction(Entity* entity) override;
		void AfterAction() override;

	private:
		const ComponentTypes MASK = (COMPONENT_TRANSFORM | COMPONENT_GEOMETRY | COMPONENT_ANIMATOR);

		void GPUComputeAABB(ComponentTransform* transform, ComponentGeometry* geometry, ComponentAnimator* animator);

		ComputeShader* minMaxVerticesShader;
		//const ShaderStorageBuffer* vertexInput;
		const ShaderStorageBuffer* minMaxOutput;
	};
}