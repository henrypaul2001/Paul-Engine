#pragma once
#include "RenderAPI.h"
#include "Shader.h"
#include "Camera.h"
#include "EditorCamera.h"
#include "RenderPipeline.h"

#include "Material.h"

namespace PaulEngine {
	class Renderer {
	public:
		static const int MAX_ACTIVE_DIR_LIGHTS = 8;
		static const int MAX_ACTIVE_POINT_LIGHTS = 8;
		static const int MAX_ACTIVE_SPOT_LIGHTS = 8;

		struct DirectionalLight // vec4 for padding
		{
			glm::vec4 Direction = glm::vec4(-0.2f, -0.5f, -0.3f, 1.0f);
			glm::vec4 Ambient = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
			glm::vec4 Diffuse = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
			glm::vec4 Specular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
			glm::mat4 LightMatrix = glm::mat4(1.0f);
		};
		struct PointLight // vec4 for padding
		{
			glm::vec4 Position = glm::vec4(0.0f, 0.0f, 0.0f, 25.0f); // w = range
			glm::vec4 Ambient = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
			glm::vec4 Diffuse = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
			glm::vec4 Specular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
			glm::vec4 ShadowData = glm::vec4(1.0f); // r = minBias, g = maxBias, b = farPlane, w = (bool)castShadows
		};
		struct SpotLight // vec4 for padding
		{
			glm::vec4 Position = glm::vec4(0.0f, 0.0f, 0.0f, 25.0f); // w = range
			glm::vec4 Direction = glm::vec4(0.0f, 0.0f, -1.0f, 25.0f); // w = cutoff

			glm::vec4 Ambient = glm::vec4(0.1f, 0.1f, 0.1f, 35.0f); // w = outer cutoff
			glm::vec4 Diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
			glm::vec4 Specular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

			glm::vec4 ShadowData = glm::vec4(1.0f); // r = (bool)castShadows, g = minBias, b = maxBias
			glm::mat4 LightMatrix = glm::mat4(1.0f);
		};

		static void Init();

		static void BeginScene(const EditorCamera& camera);
		static void BeginScene(const Camera& camera, const glm::mat4& worldTransform);
		static void BeginScene(const glm::mat4& projection, const glm::mat4& worldTransform, float gamma = 2.2f, float exposure = 1.0f);
		static void EndScene();
		static void Flush();

		static void SubmitDefaultCube(AssetHandle materialHandle, const glm::mat4& transform, DepthState depthState, FaceCulling cullState, BlendState blendState, int entityID = -1);
		static void SubmitDefaultQuad(AssetHandle materialHandle, const glm::mat4& transform, DepthState depthState, FaceCulling cullState, BlendState blendState, int entityID = -1);
		static void SubmitMesh(Ref<VertexArray> vertexArray, AssetHandle materialHandle, const glm::mat4& transform, DepthState depthState, FaceCulling cullState, BlendState blendState, int entityID = -1);

		static void SubmitDirectionalLightSource(const DirectionalLight& light);
		static void SubmitPointLightSource(const PointLight& light);
		static void SubmitSpotLightSource(const SpotLight& light);

		static void DrawDefaultCubeImmediate(Ref<Material> material, const glm::mat4& transform, DepthState depthState, FaceCulling cullState, BlendState blendState, int entityID = -1);
		static void DrawDefaultQuadImmediate(Ref<Material> material, const glm::mat4& transform, DepthState depthState, FaceCulling cullState, BlendState blendState, int entityID = -1);
		static void DrawMeshImmediate(Ref<VertexArray> vertexArray, Ref<Material> material, const glm::mat4& transform, DepthState depthState, FaceCulling cullState, BlendState blendState, int entityID = -1);

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
		static std::string ConstructPipelineStateKey(const AssetHandle material, const DepthState depthState, const FaceCulling cullState, const BlendState blendState);
	};
}