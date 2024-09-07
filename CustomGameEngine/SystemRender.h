#pragma once
#include "System.h"
#include "Camera.h"
#include "ComponentGeometry.h"
#include "ComponentTransform.h"
#include <map>
namespace Engine {
	enum PostProcessingEffect {
		NONE = 0u,
		INVERSION = 1u,
		GRAYSCALE = 2u,
		HARD_SHARPEN = 3u,
		SUBTLE_SHARPEN = 4u,
		BLUR = 5u,
		EDGE_DETECT = 6u,
		EMBOSS = 7u,
		SOBEL = 8u,
		CUSTOM_KERNEL = 9u
	};

	class SystemRender : public System
	{
	public:
		SystemRender();
		~SystemRender();

		SystemTypes Name() override { return SYSTEM_RENDER; }
		void Run(const std::vector<Entity*>& entityList) override;
		void OnAction(Entity* entity) override;
		void AfterAction() override;

		void SetPostProcess(PostProcessingEffect effect) { postProcess = effect; }
		PostProcessingEffect GetPostProcess() { return postProcess; }

		void DrawTransparentGeometry(bool useDefaultForwardShader);

		void RenderMeshes(const std::map<float, Mesh*>& meshesAndDistances, const bool transparencyPass = false, bool useDefaultForwardShader = false);
		void RenderMeshes(const std::vector<Mesh*>& meshList, const bool transparencyPass = false, bool useDefaultForwardShader = false);

		void RenderMesh(Mesh* mesh, const bool transparencyPass = false, bool useDefaultForwardShader = false);

		float PostProcessKernel[9];

		static std::map<float, Mesh*> transparentMeshes;

		Camera* GetActiveCamera() { return activeCamera; }
		void SetActiveCamera(Camera* camera) { activeCamera = camera; }
	private:
		//Camera* camera;
		const ComponentTypes MASK = (COMPONENT_TRANSFORM | COMPONENT_GEOMETRY);
		void Draw(ComponentTransform* transform, ComponentGeometry* geometry);
		void AddMeshToTransparentMeshes(ComponentTransform* transform, Mesh* mesh);

		std::vector<Shader*> shadersUsedThisFrame;

		std::map<float, ComponentGeometry*> transparentGeometry;

		PostProcessingEffect postProcess;

		Camera* activeCamera;
	};
}