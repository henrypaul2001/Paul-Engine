#pragma once
#include "RenderAPI.h"
#include "Shader.h"
#include "Camera.h"
#include "EditorCamera.h"

namespace PaulEngine{
	class Renderer {
	public:
		static void Init();

		static void BeginScene(const EditorCamera& camera);
		static void BeginScene(const Camera& camera, const glm::mat4& transform);
		static void EndScene();
		static void Flush();

		static void SubmitDefaultCube(AssetHandle materialHandle, const glm::mat4& transform, int entityID = -1);
		static void SubmitDefaultQuad(AssetHandle materialHandle, const glm::mat4& transform, int entityID = -1);
		static void SubmitMesh(Ref<VertexArray> vertexArray, AssetHandle materialHandle, const glm::mat4& transform, int entityID = -1);

		inline static RenderAPI::API GetAPI() { return RenderAPI::GetAPI(); }
	
		struct Statistics
		{
			uint32_t DrawCalls = 0;
			uint32_t MeshCount = 0;
			uint32_t MaterialCount = 0;
		};
		static void ResetStats();
		static const Statistics& GetStats();

		static void ImportShaders();

		struct DrawSubmission
		{
			Ref<VertexArray> VertexArray;
			AssetHandle MaterialHandle;
			glm::mat4 Transform;
			int EntityID;
		};

		struct RenderPass
		{
			std::vector<DrawSubmission> DrawList;
			// lighting information
			// environment info (skybox, reflection probes, shadow maps, etc)
			// render target
			// post processing steps
			// ...
		};
	};
}