#pragma once
#include "RenderAPI.h"
#include "Shader.h"
#include "Camera.h"
#include "EditorCamera.h"
#include "RenderPipeline.h"

#include "Material.h"

namespace PaulEngine{
	class Renderer {
	public:
		static void Init();

		static void BeginScene(const EditorCamera& camera);
		static void BeginScene(const Camera& camera, const glm::mat4& transform);
		static void EndScene();
		static void Flush();

		static void SubmitDefaultCube(AssetHandle materialHandle, const glm::mat4& transform, DepthState depthState, FaceCulling cullState, int entityID = -1);
		static void SubmitDefaultQuad(AssetHandle materialHandle, const glm::mat4& transform, DepthState depthState, FaceCulling cullState, int entityID = -1);
		static void SubmitMesh(Ref<VertexArray> vertexArray, AssetHandle materialHandle, const glm::mat4& transform, DepthState depthState, FaceCulling cullState, int entityID = -1);

		static void DrawDefaultCubeImmediate(Ref<Material> material, const glm::mat4& transform, DepthState depthState, FaceCulling cullState, int entityID = -1);
		static void DrawDefaultQuadImmediate(Ref<Material> material, const glm::mat4& transform, DepthState depthState, FaceCulling cullState, int entityID = -1);
		static void DrawMeshImmediate(Ref<VertexArray> vertexArray, Ref<Material> material, const glm::mat4& transform, DepthState depthState, FaceCulling cullState, int entityID = -1);

		inline static RenderAPI::API GetAPI() { return RenderAPI::GetAPI(); }
	
		struct Statistics
		{
			uint32_t DrawCalls = 0;
			uint32_t MeshCount = 0;
			uint32_t PipelineCount = 0;
		};
		static void ResetStats();
		static const Statistics& GetStats();

		static void ImportShaders();

	private:
		static std::string ConstructPipelineStateKey(const AssetHandle material, const DepthState depthState, const FaceCulling cullState);
	};
}