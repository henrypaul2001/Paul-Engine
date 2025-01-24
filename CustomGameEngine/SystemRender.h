#pragma once
#include "EntityManagerNew.h"
#include "LightManager.h"
#include "Camera.h"
#include "ComponentTransform.h"
#include "ComponentGeometry.h"
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

	class SystemRender
	{
	public:
		friend class RenderPipeline;
		SystemRender();
		~SystemRender();

		constexpr const char* SystemName() const { return "SYSTEM_RENDER"; }

		void AfterAction();

		void SetPostProcess(PostProcessingEffect effect) { postProcess = effect; }
		PostProcessingEffect GetPostProcess() const { return postProcess; }

		void RenderMeshes(const std::map<float, std::pair<Mesh*, unsigned int>>& meshesAndDistances, const bool transparencyPass = false, bool useDefaultForwardShader = false);

		void RenderMesh(const unsigned int entityID, Mesh* mesh, const bool transparencyPass = false, bool useDefaultForwardShader = false);

		float PostProcessKernel[9];

		static std::map<float, std::pair<Mesh*, unsigned int>> transparentMeshes;

		const Camera* GetActiveCamera() const { return activeCamera; }
		Camera* GetActiveCamera() { return activeCamera; }
		void SetActiveCamera(Camera* camera) { activeCamera = camera; }

	private:
		std::unordered_map<unsigned int, Shader*> shadersUsedThisFrame;

		PostProcessingEffect postProcess;

		Camera* activeCamera;

		EntityManagerNew* ecs;
		LightManager* lightManager;
	};
}