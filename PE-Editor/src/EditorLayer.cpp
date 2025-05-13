#include "EditorLayer.h"
#include <imgui.h>

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <PaulEngine/Renderer/Renderer2D.h>

#include <PaulEngine/Scene/SceneSerializer.h>
#include <PaulEngine/Utils/PlatformUtils.h>

#include <ImGuizmo.h>
#include <PaulEngine/Maths/Maths.h>

#include "PaulEngine/Renderer/Font.h"

#include "PaulEngine/Asset/SceneImporter.h"
#include "PaulEngine/Asset/MaterialImporter.h"

#include "PaulEngine/Events/SceneEvent.h"
#include "PaulEngine/Renderer/Material.h"

namespace PaulEngine
{
	struct CubemapCaptureBuffer
	{
		glm::mat4 ViewProjections[6];
		int CubemapIndex;
		float FarPlane;
	};

	void EditorLayer::CreateRenderer(FrameRenderer& out_Framerenderer)
	{
		PE_PROFILE_FUNCTION();

		// Create resources
		// ----------------
		TextureSpecification screenSpec;
		screenSpec.Width = 1280;
		screenSpec.Height = 720;
		screenSpec.GenerateMips = false;
		screenSpec.Format = ImageFormat::RGBA16F;
		screenSpec.MinFilter = ImageMinFilter::NEAREST;
		screenSpec.MagFilter = ImageMagFilter::NEAREST;
		screenSpec.Wrap_S = ImageWrap::CLAMP_TO_BORDER;
		screenSpec.Wrap_T = ImageWrap::CLAMP_TO_BORDER;
		screenSpec.Wrap_R = ImageWrap::CLAMP_TO_BORDER;
		screenSpec.Border = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		Ref<Texture2D> screenTexture = Texture2D::Create(screenSpec);
		Ref<Texture2D> alternateScreenTexture = Texture2D::Create(screenSpec);
		out_Framerenderer.AddRenderResource<RenderComponentTexture>("ScreenTexture", screenTexture);
		out_Framerenderer.AddRenderResource<RenderComponentTexture>("AlternateScreenTexture", alternateScreenTexture);

		Ref<FramebufferTexture2DAttachment> screenAttachment = FramebufferTexture2DAttachment::Create(FramebufferAttachmentPoint::Colour0, screenTexture);
		Ref<FramebufferTexture2DAttachment> alternateScreenAttachment = FramebufferTexture2DAttachment::Create(FramebufferAttachmentPoint::Colour0, alternateScreenTexture);

		out_Framerenderer.AddRenderResource<RenderComponentFBOAttachment>("ScreenAttachment", screenAttachment);
		out_Framerenderer.AddRenderResource<RenderComponentFBOAttachment>("AlternateScreenAttachment", alternateScreenAttachment);

		m_MainFramebuffer->AddColourAttachment(screenAttachment);

		// Shadow maps
		// -----------
		TextureSpecification depthSpec;
		depthSpec.Border = glm::vec4(1.0f);
		depthSpec.Width = m_ShadowWidth;
		depthSpec.Height = m_ShadowHeight;
		depthSpec.MinFilter = ImageMinFilter::NEAREST;
		depthSpec.MagFilter = ImageMagFilter::NEAREST;
		depthSpec.Wrap_S = ImageWrap::CLAMP_TO_BORDER;
		depthSpec.Wrap_T = ImageWrap::CLAMP_TO_BORDER;
		depthSpec.Wrap_R = ImageWrap::CLAMP_TO_BORDER;
		depthSpec.Format = ImageFormat::Depth32;

		Ref<Texture2DArray> dirLightShadowArray = Texture2DArray::Create(depthSpec, std::vector<Buffer>(Renderer::MAX_ACTIVE_DIR_LIGHTS));
		Ref<Texture2DArray> spotLightShadowArray = Texture2DArray::Create(depthSpec, std::vector<Buffer>(Renderer::MAX_ACTIVE_SPOT_LIGHTS));
		Ref<TextureCubemapArray> pointLightShadowArray = TextureCubemapArray::Create(depthSpec, std::vector<std::vector<Buffer>>(Renderer::MAX_ACTIVE_SPOT_LIGHTS, std::vector<Buffer>(6)));

		out_Framerenderer.AddRenderResource<RenderComponentTexture>("DirLightShadowMap", dirLightShadowArray);
		out_Framerenderer.AddRenderResource<RenderComponentTexture>("SpotLightShadowMap", spotLightShadowArray);
		out_Framerenderer.AddRenderResource<RenderComponentTexture>("PointLightShadowMap", pointLightShadowArray);

		glm::ivec2 shadowRes = { m_ShadowWidth, m_ShadowHeight };
		out_Framerenderer.AddRenderResource<RenderComponentPrimitiveType<glm::ivec2>>("ShadowResolution", shadowRes);

		//  Data
		// ------
		glm::ivec2 viewportRes = { (glm::ivec2)m_ViewportSize };
		out_Framerenderer.AddRenderResource<RenderComponentPrimitiveType<glm::ivec2>>("ViewportResolution", viewportRes);
		
		out_Framerenderer.AddRenderResource<RenderComponentPrimitiveType<bool>>("ShowColliders", m_ShowColliders);
		out_Framerenderer.AddRenderResource<RenderComponentPrimitiveType<Entity>>("SelectedEntity", Entity());

		out_Framerenderer.AddRenderResource<RenderComponentPrimitiveType<float>>("OutlineThickness", m_EntityOutlineThickness);

		out_Framerenderer.AddRenderResource<RenderComponentPrimitiveType<glm::vec4>>("OutlineColour", m_EntityOutlineColour);

		// Bloom mip chain
		// ---------------
		struct BloomMipChain
		{
			void Init(const glm::ivec2 viewportSize, const uint8_t chainLength = 6)
			{
				TextureSpecification mipSpec;
				mipSpec.Format = ImageFormat::R11FG11FB10F;
				mipSpec.MinFilter = ImageMinFilter::LINEAR;
				mipSpec.MagFilter = ImageMagFilter::LINEAR;
				mipSpec.Wrap_S = ImageWrap::CLAMP_TO_EDGE;
				mipSpec.Wrap_T = ImageWrap::CLAMP_TO_EDGE;
				mipSpec.GenerateMips = false;

				m_Chain.clear();
				m_Chain.reserve(chainLength);
				glm::vec2 mipSize = viewportSize;
				for (uint8_t i = 0; i < chainLength; i++)
				{
					mipSize *= 0.5f;
					mipSize = glm::max(mipSize, glm::vec2(1.0f, 1.0f));

					mipSpec.Width = (uint32_t)mipSize.x;
					mipSpec.Height = (uint32_t)mipSize.y;

					Ref<Texture2D> texture = Texture2D::Create(mipSpec);
					m_Chain.push_back(texture);
				}
			}

			size_t Size() const { return m_Chain.size(); }

			Ref<Texture2D> GetMipLevel(uint8_t mip) { 
				PE_CORE_ASSERT(mip < m_Chain.size(), "Index out of bounds. Index: {0}, Size: {1}", mip, m_Chain.size());
				return m_Chain[mip];
			}

		private:
			std::vector<Ref<Texture2D>> m_Chain;
		};
		BloomMipChain bloomMipChain;
		bloomMipChain.Init(m_ViewportSize, 6);

		out_Framerenderer.AddRenderResource<RenderComponentPrimitiveType<BloomMipChain>>("BloomMipChain", bloomMipChain);

		// Framebuffers
		// ------------
		FramebufferSpecification spec;
		spec.Width = m_ShadowWidth;
		spec.Height = m_ShadowHeight;
		spec.Samples = 1;

		Ref<FramebufferTexture2DArrayAttachment> dirLightShadowDepthArrayAttach = FramebufferTexture2DArrayAttachment::Create(FramebufferAttachmentPoint::Depth, dirLightShadowArray);
		Ref<Framebuffer> dirLightShadowsFramebuffer = Framebuffer::Create(spec, {}, dirLightShadowDepthArrayAttach);

		Ref<FramebufferTexture2DArrayAttachment> spotLightShadowDepthArrayAttach = FramebufferTexture2DArrayAttachment::Create(FramebufferAttachmentPoint::Depth, spotLightShadowArray);
		Ref<Framebuffer> spotLightShadowsFramebuffer = Framebuffer::Create(spec, {}, spotLightShadowDepthArrayAttach);

		Ref<FramebufferTextureCubemapArrayAttachment> pointLightShadowDepthAttach = FramebufferTextureCubemapArrayAttachment::Create(FramebufferAttachmentPoint::Depth, pointLightShadowArray);
		Ref<Framebuffer> pointLightShadowsFramebuffer = Framebuffer::Create(spec, {}, pointLightShadowDepthAttach);

		FramebufferSpecification bloomSpec;
		bloomSpec.Width = m_ViewportSize.x;
		bloomSpec.Height = m_ViewportSize.y;
		Ref<FramebufferTexture2DAttachment> bloomColourAttachment = FramebufferTexture2DAttachment::Create(FramebufferAttachmentPoint::Colour0, bloomMipChain.GetMipLevel(0));
		Ref<Framebuffer> bloomFBO = Framebuffer::Create(bloomSpec, { bloomColourAttachment });

		out_Framerenderer.AddRenderResource<RenderComponentFramebuffer>("MainFramebuffer", m_MainFramebuffer);
		out_Framerenderer.AddRenderResource<RenderComponentFramebuffer>("DirLightFramebuffer", dirLightShadowsFramebuffer);
		out_Framerenderer.AddRenderResource<RenderComponentFramebuffer>("SpotLightFramebuffer", spotLightShadowsFramebuffer);
		out_Framerenderer.AddRenderResource<RenderComponentFramebuffer>("PointLightFramebuffer", pointLightShadowsFramebuffer);
		out_Framerenderer.AddRenderResource<RenderComponentFramebuffer>("BloomFramebuffer", bloomFBO);

		// Materials
		// ---------
		Ref<EditorAssetManager> assetManager = Project::GetActive()->GetEditorAssetManager();
		std::filesystem::path engineAssetsRelativeToProjectAssets = std::filesystem::path("assets").lexically_relative(Project::GetAssetDirectory());

		AssetHandle shadowmapShaderHandle = assetManager->ImportAsset(engineAssetsRelativeToProjectAssets / "shaders/DepthShader.glsl", true);
		Ref<Material> shadowmapMaterial = CreateRef<Material>(shadowmapShaderHandle);

		Ref<UniformBuffer> cubemapDataUBO = UniformBuffer::Create(sizeof(CubemapCaptureBuffer), 3);
		AssetHandle shadowmapCubeShaderHandle = assetManager->ImportAsset(engineAssetsRelativeToProjectAssets / "shaders/DepthShaderCube.glsl", true);
		Ref<Material> shadowmapCubeMaterial = CreateRef<Material>(shadowmapCubeShaderHandle);

		AssetHandle gammaTonemapShaderHandle = assetManager->ImportAsset(engineAssetsRelativeToProjectAssets / "shaders/GammaTonemap.glsl", true);
		Ref<Material> gammaTonemapMaterial = CreateRef<Material>(gammaTonemapShaderHandle);

		AssetHandle bloomDownsampleShaderHandle = assetManager->ImportAsset(engineAssetsRelativeToProjectAssets / "shaders/MipChainDownsample.glsl", true);
		Ref<Material> mipchainDownsampleMaterial = CreateRef<Material>(bloomDownsampleShaderHandle);

		AssetHandle bloomUpsampleShaderHandle = assetManager->ImportAsset(engineAssetsRelativeToProjectAssets / "shaders/MipChainUpsample.glsl", true);
		Ref<Material> mipchainUpsampleMaterial = CreateRef<Material>(bloomUpsampleShaderHandle);

		AssetHandle bloomCombineShaderHandle = assetManager->ImportAsset(engineAssetsRelativeToProjectAssets / "shaders/MipChainBloomCombine.glsl", true);
		Ref<Material> bloomCombineMaterial = CreateRef<Material>(bloomCombineShaderHandle);

		out_Framerenderer.AddRenderResource<RenderComponentMaterial>("ShadowmapMaterial", shadowmapMaterial);
		out_Framerenderer.AddRenderResource<RenderComponentMaterial>("ShadowmapCubeMaterial", shadowmapCubeMaterial);
		out_Framerenderer.AddRenderResource<RenderComponentMaterial>("GammaTonemapMaterial", gammaTonemapMaterial);
		out_Framerenderer.AddRenderResource<RenderComponentUBO>("CubemapDataUBO", cubemapDataUBO);
		out_Framerenderer.AddRenderResource<RenderComponentMaterial>("MipChainDownsampleMaterial", mipchainDownsampleMaterial);
		out_Framerenderer.AddRenderResource<RenderComponentMaterial>("MipChainUpsampleMaterial", mipchainUpsampleMaterial);
		out_Framerenderer.AddRenderResource<RenderComponentMaterial>("BloomCombineMaterial", bloomCombineMaterial);

		// OnEvent
		// -------
		FrameRenderer::OnEventFunc eventFunc = [](Event& e, FrameRenderer* self)
		{
			EventDispatcher dispatcher = EventDispatcher(e);
			dispatcher.DispatchEvent<MainViewportResizeEvent>([self](MainViewportResizeEvent& e)->bool {
				glm::ivec2 viewportSize = glm::ivec2(e.GetWidth(), e.GetHeight());
				self->GetRenderResource<RenderComponentPrimitiveType<glm::ivec2>>("ViewportResolution")->Data = viewportSize;
				self->GetRenderResource<RenderComponentFBOAttachment>("ScreenAttachment")->Attachment->Resize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
				self->GetRenderResource<RenderComponentFBOAttachment>("AlternateScreenAttachment")->Attachment->Resize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
				self->GetRenderResource<RenderComponentPrimitiveType<BloomMipChain>>("BloomMipChain")->Data.Init(viewportSize, 6);
				self->GetRenderResource<RenderComponentFramebuffer>("BloomFramebuffer")->Framebuffer->Resize(viewportSize.x, viewportSize.y);
				return false;
			});
		};
		out_Framerenderer.SetEventFunc(eventFunc);

		// Render pass functions
		// ---------------------

		// { primitive<glm::ivec2>, material }
		RenderPass::OnRenderFunc dirLightShadowPassFunc = [](RenderPass::RenderPassContext& context, Ref<Framebuffer> targetFramebuffer, std::vector<IRenderComponent*> inputs) {
			PE_PROFILE_SCOPE("Directional light shadow map capture pass");
			Ref<Scene>& sceneContext = context.ActiveScene;
			PE_CORE_ASSERT(inputs[0], "Shadow resolution input required");
			PE_CORE_ASSERT(inputs[1], "Shadow map material input required");
			RenderComponentPrimitiveType<glm::ivec2>* shadowResInput = dynamic_cast<RenderComponentPrimitiveType<glm::ivec2>*> (inputs[0]);
			RenderComponentMaterial* shadowmapMaterial = dynamic_cast<RenderComponentMaterial*>(inputs[1]);

			RenderCommand::SetViewport({ 0, 0 }, shadowResInput->Data);

			Ref<FramebufferAttachment> depthAttachment = targetFramebuffer->GetDepthAttachment();
			PE_CORE_ASSERT(depthAttachment, "Shadow map framebuffer missing depth attachment");
			PE_CORE_ASSERT(depthAttachment->GetType() == FramebufferAttachmentType::Texture2DArray, "Shadow map framebuffer depth attachment must be texture array");

			std::vector<Entity> dirLights = std::vector<Entity>(Renderer::MAX_ACTIVE_DIR_LIGHTS);
			int dirLightsHead = 0;
			int activeLights = 0;
			auto view = sceneContext->View<ComponentDirectionalLight>();

			// Get directional light entities within maximum active lights constraint in order matching Renderer::SubmitLightSource
			for (auto entityID : view) {
				dirLights[dirLightsHead] = Entity(entityID, sceneContext.get());
				dirLightsHead = ++dirLightsHead % Renderer::MAX_ACTIVE_DIR_LIGHTS;
				activeLights = std::min(Renderer::MAX_ACTIVE_DIR_LIGHTS, ++activeLights);
			}

			// Capture shadow maps for previously gathered light sources
			for (int i = 0; i < activeLights; i++) {
				Entity entity = dirLights[i];
				ComponentTransform& transform = entity.GetComponent<ComponentTransform>();
				ComponentDirectionalLight& light = entity.GetComponent<ComponentDirectionalLight>();

				if (light.CastShadows)
				{
					FramebufferTexture2DArrayAttachment* depthArrayAttachment = dynamic_cast<FramebufferTexture2DArrayAttachment*>(depthAttachment.get());
					depthArrayAttachment->SetTargetIndex(i);
					targetFramebuffer->SetDepthAttachment(depthAttachment);
					RenderCommand::Clear();

					glm::mat4 transformMatrix = transform.GetTransform();
					glm::mat3 rotationMatrix = glm::mat3(transformMatrix);

					rotationMatrix[0] = glm::normalize(rotationMatrix[0]);
					rotationMatrix[1] = glm::normalize(rotationMatrix[1]);
					rotationMatrix[2] = glm::normalize(rotationMatrix[2]);

					glm::vec3 direction = glm::normalize(rotationMatrix * glm::vec3(0.0f, 0.0f, 1.0f));

					float shadowSize = light.ShadowMapProjectionSize;
					float nearClip = light.ShadowMapNearClip;
					float farClip = light.ShadowMapFarClip;

					glm::mat4 cameraTransform = glm::inverse(glm::lookAt(-direction * light.ShadowMapCameraDistance, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
					SceneCamera cam = SceneCamera(SCENE_CAMERA_ORTHOGRAPHIC);
					cam.SetOrthographic(shadowSize, (float)shadowResInput->Data.x / (float)shadowResInput->Data.y, nearClip, farClip);

					Renderer::BeginScene(cam, cameraTransform);

					{
						PE_PROFILE_SCOPE("Submit Mesh");
						auto view = sceneContext->View<ComponentTransform, ComponentMeshRenderer>();
						for (auto entityID : view) {
							auto [transform, mesh] = view.get<ComponentTransform, ComponentMeshRenderer>(entityID);
							Renderer::DrawDefaultCubeImmediate(shadowmapMaterial->Material, transform.GetTransform(), mesh.DepthState, mesh.CullState, false, (int)entityID);
						}
					}

					Renderer::EndScene();
				}
			}
			};

		// { primitive<glm::ivec2>, material }
		RenderPass::OnRenderFunc spotLightShadowPassFunc = [](RenderPass::RenderPassContext& context, Ref<Framebuffer> targetFramebuffer, std::vector<IRenderComponent*> inputs) {
			PE_PROFILE_SCOPE("Spot light shadow map capture pass");
			Ref<Scene>& sceneContext = context.ActiveScene;
			PE_CORE_ASSERT(inputs[0], "Shadow resolution input required");
			PE_CORE_ASSERT(inputs[1], "Shadow map material input required");
			RenderComponentPrimitiveType<glm::ivec2>* shadowResInput = dynamic_cast<RenderComponentPrimitiveType<glm::ivec2>*>(inputs[0]);
			RenderComponentMaterial* shadowmapMaterial = dynamic_cast<RenderComponentMaterial*>(inputs[1]);

			RenderCommand::SetViewport({ 0, 0 }, shadowResInput->Data);

			Ref<FramebufferAttachment> depthAttachment = targetFramebuffer->GetDepthAttachment();
			PE_CORE_ASSERT(depthAttachment, "Shadow map framebuffer missing depth attachment");
			PE_CORE_ASSERT(depthAttachment->GetType() == FramebufferAttachmentType::Texture2DArray, "Shadow map framebuffer depth attachment must be texture array");

			std::vector<Entity> spotLights = std::vector<Entity>(Renderer::MAX_ACTIVE_SPOT_LIGHTS);
			int spotLightsHead = 0;
			int activeLights = 0;
			auto view = sceneContext->View<ComponentSpotLight>();

			// Get spot light entities within maximum active lights constraint in order matching Renderer::SubmitLightSource
			for (auto entityID : view) {
				spotLights[spotLightsHead] = Entity(entityID, sceneContext.get());
				spotLightsHead = ++spotLightsHead % Renderer::MAX_ACTIVE_SPOT_LIGHTS;
				activeLights = std::min(Renderer::MAX_ACTIVE_SPOT_LIGHTS, ++activeLights);
			}

			// Capture shadow maps for previously gathered light sources
			for (int i = 0; i < activeLights; i++) {
				Entity entity = spotLights[i];
				ComponentTransform& transform = entity.GetComponent<ComponentTransform>();
				ComponentSpotLight& light = entity.GetComponent<ComponentSpotLight>();

				if (light.CastShadows)
				{
					FramebufferTexture2DArrayAttachment* depthArrayAttachment = dynamic_cast<FramebufferTexture2DArrayAttachment*>(depthAttachment.get());
					depthArrayAttachment->SetTargetIndex(i);
					targetFramebuffer->SetDepthAttachment(depthAttachment);
					RenderCommand::Clear();

					glm::mat4 transformMatrix = transform.GetTransform();
					glm::mat3 rotationMatrix = glm::mat3(transformMatrix);

					rotationMatrix[0] = glm::normalize(rotationMatrix[0]);
					rotationMatrix[1] = glm::normalize(rotationMatrix[1]);
					rotationMatrix[2] = glm::normalize(rotationMatrix[2]);

					glm::vec3 position = transform.Position();
					glm::vec3 direction = glm::normalize(rotationMatrix * glm::vec3(0.0f, 0.0f, -1.0f));

					float nearClip = light.ShadowMapNearClip;
					float farClip = light.ShadowMapFarClip;

					glm::mat4 cameraTransform = glm::inverse(glm::lookAt(position, position + direction, glm::vec3(0.0f, 1.0f, 0.0f)));
					SceneCamera cam = SceneCamera(SCENE_CAMERA_PERSPECTIVE);
					cam.SetPerspective(90.0f, (float)shadowResInput->Data.x / (float)shadowResInput->Data.y, nearClip, farClip);

					Renderer::BeginScene(cam, cameraTransform);

					{
						PE_PROFILE_SCOPE("Submit Mesh");
						auto view = sceneContext->View<ComponentTransform, ComponentMeshRenderer>();
						for (auto entityID : view) {
							auto [transform, mesh] = view.get<ComponentTransform, ComponentMeshRenderer>(entityID);
							Renderer::DrawDefaultCubeImmediate(shadowmapMaterial->Material, transform.GetTransform(), mesh.DepthState, mesh.CullState, false, (int)entityID);
						}
					}

					Renderer::EndScene();
				}
			}
			};

		// { primitive<glm::ivec2>, material, ubo }
		RenderPass::OnRenderFunc pointLightShadowFunc = [](RenderPass::RenderPassContext& context, Ref<Framebuffer> targetFramebuffer, std::vector<IRenderComponent*> inputs) {
			PE_PROFILE_SCOPE("Point light shadow map capture pass");
			Ref<Scene>& sceneContext = context.ActiveScene;
			PE_CORE_ASSERT(inputs[0], "Shadow resolution input required");
			PE_CORE_ASSERT(inputs[1], "Shadow map material input required");
			PE_CORE_ASSERT(inputs[2], "Cubemap data UBO input required");
			RenderComponentPrimitiveType<glm::ivec2>* shadowResInput = dynamic_cast<RenderComponentPrimitiveType<glm::ivec2>*> (inputs[0]);
			RenderComponentMaterial* shadowmapMaterial = dynamic_cast<RenderComponentMaterial*>(inputs[1]);
			RenderComponentUBO* uboInput = dynamic_cast<RenderComponentUBO*>(inputs[2]);

			RenderCommand::SetViewport({ 0, 0 }, shadowResInput->Data);

			Ref<FramebufferAttachment> depthAttachment = targetFramebuffer->GetDepthAttachment();
			PE_CORE_ASSERT(depthAttachment, "Shadow map framebuffer missing depth attachment");
			PE_CORE_ASSERT(depthAttachment->GetType() == FramebufferAttachmentType::TextureCubemapArray, "Shadow map framebuffer depth attachment must be cubemap array");

			std::vector<Entity> pointLights = std::vector<Entity>(Renderer::MAX_ACTIVE_POINT_LIGHTS);
			int pointLightsHead = 0;
			int activeLights = 0;
			auto view = sceneContext->View<ComponentPointLight>();

			// Get directional light entities within maximum active lights constraint in order matching Renderer::SubmitLightSource
			for (auto entityID : view) {
				pointLights[pointLightsHead] = Entity(entityID, sceneContext.get());
				pointLightsHead = ++pointLightsHead % Renderer::MAX_ACTIVE_POINT_LIGHTS;
				activeLights = std::min(Renderer::MAX_ACTIVE_POINT_LIGHTS, ++activeLights);
			}

			FramebufferTextureCubemapArrayAttachment* cubemapArrayAttachment = dynamic_cast<FramebufferTextureCubemapArrayAttachment*>(depthAttachment.get());
			cubemapArrayAttachment->SetTargetIndex(0);
			cubemapArrayAttachment->SetTargetFace((CubemapFace)0);
			cubemapArrayAttachment->BindAsLayered = false;
			targetFramebuffer->SetDepthAttachment(depthAttachment);
			RenderCommand::Clear();

			// Capture shadow maps for previously gathered light sources
			for (int i = 0; i < activeLights; i++) {
				Entity entity = pointLights[i];
				ComponentTransform& transform = entity.GetComponent<ComponentTransform>();
				ComponentPointLight& light = entity.GetComponent<ComponentPointLight>();

				if (light.CastShadows)
				{
					float nearClip = light.ShadowMapNearClip;
					float farClip = light.ShadowMapFarClip;

					glm::mat4 transformMatrix = transform.GetTransform();
					glm::vec3 position = transform.Position();

					CubemapCaptureBuffer cubemapBuffer;
					glm::mat4 lightProjection = glm::perspective(glm::radians(90.0f), (float)shadowResInput->Data.x / (float)shadowResInput->Data.y, nearClip, farClip);
					cubemapBuffer.ViewProjections[0] = lightProjection * glm::lookAt(position, position + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));  // right
					cubemapBuffer.ViewProjections[1] = lightProjection * glm::lookAt(position, position + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)); // left
					cubemapBuffer.ViewProjections[2] = lightProjection * glm::lookAt(position, position + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));   // up
					cubemapBuffer.ViewProjections[3] = lightProjection * glm::lookAt(position, position + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)); // down
					cubemapBuffer.ViewProjections[4] = lightProjection * glm::lookAt(position, position + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));  // back
					cubemapBuffer.ViewProjections[5] = lightProjection * glm::lookAt(position, position + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)); // front

					cubemapBuffer.CubemapIndex = i;
					cubemapBuffer.FarPlane = farClip;

					uboInput->UBO->SetData(&cubemapBuffer, sizeof(cubemapBuffer), 0);
					uboInput->UBO->Bind(3);

					SceneCamera cam = SceneCamera(SCENE_CAMERA_PERSPECTIVE);
					cam.SetPerspective(90.0f, (float)shadowResInput->Data.x / (float)shadowResInput->Data.y, nearClip, farClip);
					Renderer::BeginScene(cam, transformMatrix);

					{
						PE_PROFILE_SCOPE("Submit Mesh");
						auto view = sceneContext->View<ComponentTransform, ComponentMeshRenderer>();
						for (auto entityID : view) {
							auto [transform, mesh] = view.get<ComponentTransform, ComponentMeshRenderer>(entityID);
							Renderer::DrawDefaultCubeImmediate(shadowmapMaterial->Material, transform.GetTransform(), mesh.DepthState, mesh.CullState, false, (int)entityID);
						}
					}

					Renderer::EndScene();
				}
			}
			};

		// { primitive<glm::ivec2>, Attachment }
		RenderPass::OnRenderFunc scene2DPass = [](RenderPass::RenderPassContext& context, Ref<Framebuffer> targetFramebuffer, std::vector<IRenderComponent*> inputs) {
			PE_PROFILE_SCOPE("Scene 2D Render Pass");
			Ref<Scene>& sceneContext = context.ActiveScene;
			Ref<Camera> activeCamera = context.ActiveCamera;
			const glm::mat4& cameraWorldTransform = context.CameraWorldTransform;
			PE_CORE_ASSERT(inputs[0], "Viewport resolution input required");
			PE_CORE_ASSERT(inputs[1], "Framebuffer attachment input required");
			RenderComponentPrimitiveType<glm::ivec2>* viewportResInput = dynamic_cast<RenderComponentPrimitiveType<glm::ivec2>*> (inputs[0]);
			RenderComponentFBOAttachment* screenTextureInput = dynamic_cast<RenderComponentFBOAttachment*>(inputs[1]);

			FramebufferTexture2DAttachment* currentAttachment = dynamic_cast<FramebufferTexture2DAttachment*>(targetFramebuffer->GetAttachment(FramebufferAttachmentPoint::Colour0).get());
			FramebufferTexture2DAttachment* targetAttachment = dynamic_cast<FramebufferTexture2DAttachment*>(screenTextureInput->Attachment.get());
			if (currentAttachment->GetTexture() != targetAttachment->GetTexture())
			{
				targetFramebuffer->AddColourAttachment(screenTextureInput->Attachment);
			}

			targetFramebuffer->SetDrawBuffers();
			RenderCommand::SetViewport({ 0.0f, 0.0f }, viewportResInput->Data);
			RenderCommand::SetClearColour(glm::vec4(0.01f, 0.01f, 0.01f, 1.0f));
			RenderCommand::Clear();

			if (activeCamera && sceneContext) {
				Renderer2D::BeginScene(activeCamera->GetProjection(), cameraWorldTransform, activeCamera->GetGamma(), activeCamera->GetExposure());
				{
					PE_PROFILE_SCOPE("Draw Quads");
					auto group = sceneContext->Group<ComponentTransform>(entt::get<Component2DSprite>);
					for (auto entityID : group) {
						auto [transform, sprite] = group.get<ComponentTransform, Component2DSprite>(entityID);
						if (sprite.TextureAtlas) {
							Renderer2D::DrawQuad(transform.GetTransform(), sprite.TextureAtlas, sprite.SelectedSubTextureName, sprite.Colour, (int)entityID);
						}
						else if (sprite.Texture) {
							Renderer2D::DrawQuad(transform.GetTransform(), sprite.Texture, sprite.TextureScale, sprite.Colour, (int)entityID);
						}
						else {
							Renderer2D::DrawQuad(transform.GetTransform(), sprite.Colour, (int)entityID);
						}
					}
				}

				{
					PE_PROFILE_SCOPE("Draw Circles");
					auto view = sceneContext->View<ComponentTransform, Component2DCircle>();
					for (auto entityID : view) {
						auto [transform, circle] = view.get<ComponentTransform, Component2DCircle>(entityID);

						Renderer2D::DrawCircle(transform.GetTransform(), circle.Colour, circle.Thickness, circle.Fade, (int)entityID);
					}
				}

				{
					PE_PROFILE_SCOPE("Draw Text");
					auto view = sceneContext->View<ComponentTransform, ComponentTextRenderer>();
					for (auto entityID : view) {
						auto [transform, text] = view.get<ComponentTransform, ComponentTextRenderer>(entityID);

						Renderer2D::TextParams params;
						params.Colour = text.Colour;
						params.Kerning = text.Kerning;
						params.LineSpacing = text.LineSpacing;

						Renderer2D::DrawString(text.TextString, text.Font, transform.GetTransform(), params, (int)entityID);
					}
				}

				Renderer2D::EndScene();
			}
			};

		// { primitive<glm::ivec2>, primitive<glm::ivec2>, Texture, Texture, Texture, Attachment }
		RenderPass::OnRenderFunc scene3DPass = [](RenderPass::RenderPassContext& context, Ref<Framebuffer> targetFramebuffer, std::vector<IRenderComponent*> inputs) {
			PE_PROFILE_SCOPE("Scene 3D Render Pass");
			Ref<Scene>& sceneContext = context.ActiveScene;
			Ref<Camera> activeCamera = context.ActiveCamera;
			const glm::mat4& cameraWorldTransform = context.CameraWorldTransform;
			PE_CORE_ASSERT(inputs[0], "Viewport resolution input required");
			PE_CORE_ASSERT(inputs[1], "Shadow resolution input required");
			PE_CORE_ASSERT(inputs[2], "Dir light shadow map input required");
			PE_CORE_ASSERT(inputs[3], "Spot light shadow map input required");
			PE_CORE_ASSERT(inputs[4], "Point light shadow map input required");
			PE_CORE_ASSERT(inputs[5], "Framebuffer attachment input required");
			RenderComponentPrimitiveType<glm::ivec2>* viewportResInput = dynamic_cast<RenderComponentPrimitiveType<glm::ivec2>*>(inputs[0]);
			RenderComponentPrimitiveType<glm::ivec2>* shadowResInput = dynamic_cast<RenderComponentPrimitiveType<glm::ivec2>*>(inputs[1]);
			RenderComponentTexture* dirLightShadowInput = dynamic_cast<RenderComponentTexture*>(inputs[2]);
			RenderComponentTexture* spotLightShadowInput = dynamic_cast<RenderComponentTexture*>(inputs[3]);
			RenderComponentTexture* pointLightShadowInput = dynamic_cast<RenderComponentTexture*>(inputs[4]);
			RenderComponentFBOAttachment* screenTextureInput = dynamic_cast<RenderComponentFBOAttachment*>(inputs[5]);

			FramebufferTexture2DAttachment* currentAttachment = dynamic_cast<FramebufferTexture2DAttachment*>(targetFramebuffer->GetAttachment(FramebufferAttachmentPoint::Colour0).get());
			FramebufferTexture2DAttachment* targetAttachment = dynamic_cast<FramebufferTexture2DAttachment*>(screenTextureInput->Attachment.get());
			if (currentAttachment->GetTexture() != targetAttachment->GetTexture())
			{
				targetFramebuffer->AddColourAttachment(screenTextureInput->Attachment);
			}

			RenderCommand::SetViewport({ 0, 0 }, viewportResInput->Data);

			if (activeCamera && sceneContext) {
				Renderer::BeginScene(activeCamera->GetProjection(), cameraWorldTransform, activeCamera->GetGamma(), activeCamera->GetExposure());

				{
					PE_PROFILE_SCOPE("Submit Mesh");
					auto view = sceneContext->View<ComponentTransform, ComponentMeshRenderer>();
					for (auto entityID : view) {
						auto [transform, mesh] = view.get<ComponentTransform, ComponentMeshRenderer>(entityID);
						Renderer::SubmitDefaultCube(mesh.MaterialHandle, transform.GetTransform(), mesh.DepthState, mesh.CullState, true, (int)entityID);
					}
				}

				{
					PE_PROFILE_SCOPE("Submit lights");
					{
						PE_PROFILE_SCOPE("Directional lights");
						auto view = sceneContext->View<ComponentTransform, ComponentDirectionalLight>();
						for (auto entityID : view) {
							auto [transform, light] = view.get<ComponentTransform, ComponentDirectionalLight>(entityID);
							glm::mat4 transformMatrix = transform.GetTransform();
							glm::mat3 rotationMatrix = glm::mat3(transformMatrix);

							rotationMatrix[0] = glm::normalize(rotationMatrix[0]);
							rotationMatrix[1] = glm::normalize(rotationMatrix[1]);
							rotationMatrix[2] = glm::normalize(rotationMatrix[2]);

							Renderer::DirectionalLight lightSource;
							lightSource.Direction = glm::vec4(glm::normalize(rotationMatrix * glm::vec3(0.0f, 0.0f, 1.0f)), (float)light.CastShadows);
							lightSource.Diffuse = glm::vec4(light.Diffuse, light.ShadowMinBias);
							lightSource.Specular = glm::vec4(light.Specular, light.ShadowMaxBias);
							lightSource.Ambient = glm::vec4(light.Ambient, light.ShadowMapCameraDistance);

							float shadowSize = light.ShadowMapProjectionSize;

							glm::mat4 lightView = glm::lookAt(-glm::vec3(lightSource.Direction) * light.ShadowMapCameraDistance, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
							float aspectRatio = (float)shadowResInput->Data.x / (float)shadowResInput->Data.y;
							float orthoLeft = -shadowSize * aspectRatio * 0.5f;
							float orthoRight = shadowSize * aspectRatio * 0.5f;
							float orthoBottom = -shadowSize * 0.5f;
							float orthoTop = shadowSize * 0.5f;

							glm::mat4 lightProjection = glm::ortho(orthoLeft, orthoRight, orthoBottom, orthoTop, light.ShadowMapNearClip, light.ShadowMapFarClip);
							lightSource.LightMatrix = lightProjection * lightView;

							Renderer::SubmitDirectionalLightSource(lightSource);
						}
					}

					{
						PE_PROFILE_SCOPE("Point lights");
						auto view = sceneContext->View<ComponentTransform, ComponentPointLight>();
						for (auto entityID : view) {
							auto [transform, light] = view.get<ComponentTransform, ComponentPointLight>(entityID);
							glm::vec4 position = glm::vec4(transform.Position(), 1.0f);
							Renderer::PointLight lightSource;
							lightSource.Position = position;
							lightSource.Position.w = light.Radius;
							lightSource.Diffuse = glm::vec4(light.Diffuse, 1.0f);
							lightSource.Specular = glm::vec4(light.Specular, 1.0f);
							lightSource.Ambient = glm::vec4(light.Ambient, 1.0f);
							lightSource.ShadowData = glm::vec4(light.ShadowMinBias, light.ShadowMaxBias, light.ShadowMapFarClip, (float)light.CastShadows);
							Renderer::SubmitPointLightSource(lightSource);
						}
					}

					{
						PE_PROFILE_SCOPE("Spot lights");
						auto view = sceneContext->View<ComponentTransform, ComponentSpotLight>();
						for (auto entityID : view) {
							auto [transform, light] = view.get<ComponentTransform, ComponentSpotLight>(entityID);
							glm::mat3 rotationMatrix = glm::mat3(transform.GetTransform());

							rotationMatrix[0] = glm::normalize(rotationMatrix[0]);
							rotationMatrix[1] = glm::normalize(rotationMatrix[1]);
							rotationMatrix[2] = glm::normalize(rotationMatrix[2]);

							glm::vec3 position = transform.Position();
							glm::vec3 direction = rotationMatrix * glm::vec3(0.0f, 0.0f, -1.0f);

							Renderer::SpotLight lightSource;
							lightSource.Position = glm::vec4(position, light.Range);
							lightSource.Direction = glm::vec4(direction, glm::cos(glm::radians(light.InnerCutoff)));
							lightSource.Diffuse = glm::vec4(light.Diffuse, 1.0f);
							lightSource.Specular = glm::vec4(light.Specular, 1.0f);
							lightSource.Ambient = glm::vec4(light.Ambient, glm::cos(glm::radians(light.OuterCutoff)));
							lightSource.ShadowData = glm::vec4((bool)light.CastShadows, light.ShadowMinBias, light.ShadowMaxBias, 1.0f);

							glm::mat4 lightView = glm::lookAt(position, position + direction, glm::vec3(0.0f, 1.0f, 0.0f));
							glm::mat4 projection = glm::perspective(glm::radians(90.0f), (float)shadowResInput->Data.x / (float)shadowResInput->Data.y, light.ShadowMapNearClip, light.ShadowMapFarClip);
							lightSource.LightMatrix = projection * lightView;

							Renderer::SubmitSpotLightSource(lightSource);
						}
					}
				}

				if (dirLightShadowInput) {
					PE_CORE_ASSERT(dirLightShadowInput->Texture->GetType() == AssetType::Texture2DArray, "Invalid directional light shadow map type");
					dirLightShadowInput->Texture->Bind(4);
				}
				if (spotLightShadowInput) {
					PE_CORE_ASSERT(spotLightShadowInput->Texture->GetType() == AssetType::Texture2DArray, "Invalid spot light shadow map type");
					spotLightShadowInput->Texture->Bind(5);
				}
				if (pointLightShadowInput) {
					PE_CORE_ASSERT(pointLightShadowInput->Texture->GetType() == AssetType::TextureCubemapArray, "Invalid point light shadow map type");
					pointLightShadowInput->Texture->Bind(6);
				}

				Renderer::EndScene();
			}
			};

		// { primitive<glm::ivec2>, primitive<BloomMipChain>, Material, Attachment }
		RenderPass::OnRenderFunc bloomDownsamplePass = [](RenderPass::RenderPassContext& context, Ref<Framebuffer> targetFramebuffer, std::vector<IRenderComponent*> inputs)
		{
			PE_PROFILE_SCOPE("Bloom Downsample Pass");
			PE_CORE_ASSERT(inputs[0], "Viewport resolution input required");
			PE_CORE_ASSERT(inputs[1], "Downsample mip chain input required");
			PE_CORE_ASSERT(inputs[2], "Downsample material input required");
			PE_CORE_ASSERT(inputs[3], "Source attachment input required");
			Ref<Camera> activeCamera = context.ActiveCamera;
			const glm::mat4& cameraWorldTransform = context.CameraWorldTransform;
			RenderComponentPrimitiveType<glm::ivec2>* viewportResInput = dynamic_cast<RenderComponentPrimitiveType<glm::ivec2>*>(inputs[0]);
			RenderComponentPrimitiveType<BloomMipChain>* mipChainInput = dynamic_cast<RenderComponentPrimitiveType<BloomMipChain>*>(inputs[1]);
			RenderComponentMaterial* downsampleMaterialInput = dynamic_cast<RenderComponentMaterial*>(inputs[2]);
			RenderComponentFBOAttachment* sourceAttachmentInput = dynamic_cast<RenderComponentFBOAttachment*>(inputs[3]);
			FramebufferTexture2DAttachment* textureAttachment = dynamic_cast<FramebufferTexture2DAttachment*>(sourceAttachmentInput->Attachment.get());

			glm::vec2 srcResolution = viewportResInput->Data;
			const float threshold = 1.0f;
			const float softThreshold = 0.5f;
			int FirstIteration = 1;
			const float gamma = activeCamera->GetGamma();

			Ref<Material> downsampleMaterial = downsampleMaterialInput->Material;
			UniformBufferStorage* uboStorage = downsampleMaterial->GetParameter<UBOShaderParameterTypeStorage>("DownsampleData")->UBO().get();
			uboStorage->SetLocalData("SourceResolution", srcResolution);
			uboStorage->SetLocalData("Threshold", threshold);
			uboStorage->SetLocalData("SoftThreshold", softThreshold);
			uboStorage->SetLocalData<int>("FirstIteration", FirstIteration);
			uboStorage->SetLocalData("Gamma", gamma);

			// Progressively downsample through mip chain
			BloomMipChain& mipChain = mipChainInput->Data;
			Texture* previousTexture = textureAttachment->GetTexture().get();
			for (int i = 0; i < mipChain.Size(); i++)
			{
				Ref<Texture2D> mip = mipChain.GetMipLevel(i);
				if (i > 0)
				{
					uboStorage->SetLocalData<int>("FirstIteration", 0);
				}

				const TextureSpecification& mipSpec = mip->GetSpecification();
				RenderCommand::SetViewport({ 0, 0 }, { mipSpec.Width, mipSpec.Height });

				Ref<FramebufferTexture2DAttachment> attachment = FramebufferTexture2DAttachment::Create(FramebufferAttachmentPoint::Colour0, mip);
				targetFramebuffer->AddColourAttachment(attachment);
				RenderCommand::Clear();

				previousTexture->Bind();
				Renderer::BeginScene(activeCamera->GetProjection(), cameraWorldTransform, activeCamera->GetGamma(), activeCamera->GetExposure());
				Renderer::DrawDefaultQuadImmediate(downsampleMaterial, glm::mat4(1.0f), { DepthFunc::ALWAYS, true, true }, FaceCulling::BACK, false, -1);
				Renderer::EndScene();

				// Set resolution for next iteration to the currently written mip
				uboStorage->SetLocalData("SourceResolution", glm::vec2(mipSpec.Width, mipSpec.Height));
				previousTexture = mip.get();
			}
		};

		// { primitive<bool>, primitive<Entity>, primitive<float>, primitive<glm::vec4>, Attachment }
		RenderPass::OnRenderFunc debugOverlayPass = [](RenderPass::RenderPassContext& context, Ref<Framebuffer> targetFramebuffer, std::vector<IRenderComponent*> inputs) {
			PE_PROFILE_SCOPE("Debug overlay render pass");
			Ref<Scene>& sceneContext = context.ActiveScene;
			Ref<Camera> activeCamera = context.ActiveCamera;
			const glm::mat4& cameraWorldTransform = context.CameraWorldTransform;
			PE_CORE_ASSERT(inputs[0], "Show colliders flag input required");
			PE_CORE_ASSERT(inputs[1], "Selected entity input required");
			PE_CORE_ASSERT(inputs[2], "Entity outline thickness input required");
			PE_CORE_ASSERT(inputs[3], "Entity outline colour input required");
			PE_CORE_ASSERT(inputs[4], "Framebuffer attachment input required");
			RenderComponentPrimitiveType<bool>* showCollidersInput = dynamic_cast<RenderComponentPrimitiveType<bool>*>(inputs[0]);
			RenderComponentPrimitiveType<Entity>* selectedEntityInput = dynamic_cast<RenderComponentPrimitiveType<Entity>*>(inputs[1]);
			RenderComponentPrimitiveType<float>* entityOutlineThicknessInput = dynamic_cast<RenderComponentPrimitiveType<float>*>(inputs[2]);
			RenderComponentPrimitiveType<glm::vec4>* entityOutlineColourInput = dynamic_cast<RenderComponentPrimitiveType<glm::vec4>*>(inputs[3]);
			RenderComponentFBOAttachment* screenTextureInput = dynamic_cast<RenderComponentFBOAttachment*>(inputs[4]);

			FramebufferTexture2DAttachment* currentAttachment = dynamic_cast<FramebufferTexture2DAttachment*>(targetFramebuffer->GetAttachment(FramebufferAttachmentPoint::Colour0).get());
			FramebufferTexture2DAttachment* targetAttachment = dynamic_cast<FramebufferTexture2DAttachment*>(screenTextureInput->Attachment.get());
			if (currentAttachment->GetTexture() != targetAttachment->GetTexture())
			{
				targetFramebuffer->AddColourAttachment(screenTextureInput->Attachment);
			}

			if (activeCamera && sceneContext)
			{
				Renderer2D::BeginScene(activeCamera->GetProjection(), cameraWorldTransform, activeCamera->GetGamma(), activeCamera->GetExposure(), FaceCulling::NONE, {DepthFunc::ALWAYS, true, true});

				if (showCollidersInput->Data)
				{
					// Box colliders
					auto boxView = sceneContext->View<ComponentTransform, ComponentBoxCollider2D>();
					for (auto entityID : boxView)
					{
						auto [transform, box] = boxView.get<ComponentTransform, ComponentBoxCollider2D>(entityID);

						glm::vec3 position = glm::vec3(glm::vec2(transform.Position()), 0.01f);
						glm::vec3 scale = transform.Scale() * (glm::vec3(box.Size() * 2.0f, 1.0f));
						glm::mat4 transformation = glm::translate(glm::mat4(1.0f), position);
						transformation = glm::rotate(transformation, transform.Rotation().z, glm::vec3(0.0, 0.0, 1.0f));
						transformation = glm::scale(transformation, scale);

						Renderer2D::SetLineWidth(0.01f);
						Renderer2D::DrawRect(transformation, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), (int)entityID);
					}

					// Circle colliders
					auto circleView = sceneContext->View<ComponentTransform, ComponentCircleCollider2D>();
					for (auto entityID : circleView)
					{
						auto [transform, circle] = circleView.get<ComponentTransform, ComponentCircleCollider2D>(entityID);

						glm::vec3 position = glm::vec3(glm::vec2(transform.Position()), 0.01f);
						glm::vec3 scale = transform.Scale() * (circle.Radius() * 2.0f);
						glm::mat4 transformation = glm::translate(glm::mat4(1.0f), position);
						transformation = glm::scale(transformation, scale);

						Renderer2D::DrawCircle(transformation, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), 0.01f, 0.0f, (int)entityID);
					}
				}
				Renderer2D::Flush();

				Entity selectedEntity = selectedEntityInput->Data;
				float outlineThickness = entityOutlineThicknessInput->Data;
				glm::vec4 outlineColour = entityOutlineColourInput->Data;
				if (selectedEntity.BelongsToScene(sceneContext) && selectedEntity)
				{
					// Entity outline
					ComponentTransform transformCopy = selectedEntity.GetComponent<ComponentTransform>();
					transformCopy.SetPosition(transformCopy.Position() + glm::vec3(0.0f, 0.0f, 0.01f));
					Renderer2D::SetLineWidth(outlineThickness);
					Renderer2D::DrawRect(transformCopy.GetTransform(), outlineColour);

					// Point light radius
					if (selectedEntity.HasComponent<ComponentPointLight>())
					{
						ComponentTransform& transformComponent = selectedEntity.GetComponent<ComponentTransform>();
						ComponentPointLight& pointLight = selectedEntity.GetComponent<ComponentPointLight>();
						float radius = pointLight.Radius;
						float thickness = 0.005f;
						float fade = 0.0f;

						glm::mat4 transform = glm::mat4(1.0f);
						transform = glm::translate(transform, transformComponent.Position());
						transform = glm::scale(transform, glm::vec3(radius, radius, 1.0f));
						Renderer2D::DrawCircle(transform, glm::vec4(1.0f), thickness, fade);

						transform = glm::mat4(1.0f);
						transform = glm::translate(transform, transformComponent.Position());
						transform = glm::rotate(transform, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
						transform = glm::scale(transform, glm::vec3(radius, radius, 1.0f));
						Renderer2D::DrawCircle(transform, glm::vec4(1.0f), thickness, fade);

						transform = glm::mat4(1.0f);
						transform = glm::translate(transform, transformComponent.Position());
						transform = glm::rotate(transform, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
						transform = glm::scale(transform, glm::vec3(radius, radius, 1.0f));
						Renderer2D::DrawCircle(transform, glm::vec4(1.0f), thickness, fade);
					}

					// Spot light
					if (selectedEntity.HasComponent<ComponentSpotLight>())
					{
						ComponentTransform& transformComponent = selectedEntity.GetComponent<ComponentTransform>();
						ComponentSpotLight& spotLight = selectedEntity.GetComponent<ComponentSpotLight>();
						glm::vec3 position = transformComponent.Position();
						glm::quat rotationQuat = glm::quat(transformComponent.Rotation());
						float outerRadius = glm::tan(glm::radians(spotLight.OuterCutoff)) * spotLight.Range;
						float innerRadius = glm::tan(glm::radians(spotLight.InnerCutoff)) * spotLight.Range;
						float thickness = 0.005f;
						float fade = 0.0f;

						glm::mat4 transform = glm::mat4(1.0f);
						transform = glm::translate(transform, position);
						glm::mat4 rotation = glm::toMat4(rotationQuat);
						transform *= rotation;
						transform = glm::translate(transform, glm::vec3(0.0, 0.0f, -spotLight.Range));

						glm::vec3 forward = glm::normalize(glm::rotate(rotationQuat, glm::vec3(0.0f, 0.0f, -1.0f)));
						glm::vec3 right = glm::normalize(glm::rotate(rotationQuat, glm::vec3(1.0f, 0.0f, 0.0f)));
						glm::vec3 up = glm::cross(forward, right);

						Renderer2D::SetLineWidth(thickness);

						// Outer cone
						glm::mat4 outerCircleTransform = glm::scale(transform, glm::vec3(outerRadius, outerRadius, 1.0f));
						Renderer2D::DrawCircle(outerCircleTransform, glm::vec4(1.0f), thickness, fade);

						glm::vec3 line = forward * spotLight.Range + right * outerRadius * 0.5f;
						Renderer2D::DrawLine(position, position + line, glm::vec4(1.0f));

						line = forward * spotLight.Range - right * outerRadius * 0.5f;
						Renderer2D::DrawLine(position, position + line, glm::vec4(1.0f));

						line = forward * spotLight.Range + up * outerRadius * 0.5f;
						Renderer2D::DrawLine(position, position + line, glm::vec4(1.0f));

						line = forward * spotLight.Range - up * outerRadius * 0.5f;
						Renderer2D::DrawLine(position, position + line, glm::vec4(1.0f));

						// Inner cone
						glm::mat4 innerCircleTransform = glm::scale(transform, glm::vec3(innerRadius, innerRadius, 1.0f));
						Renderer2D::DrawCircle(innerCircleTransform, glm::vec4(1.0f), thickness, fade);

						line = forward * spotLight.Range + right * innerRadius * 0.5f;
						Renderer2D::DrawLine(position, position + line, glm::vec4(1.0f));

						line = forward * spotLight.Range - right * innerRadius * 0.5f;
						Renderer2D::DrawLine(position, position + line, glm::vec4(1.0f));

						line = forward * spotLight.Range + up * innerRadius * 0.5f;
						Renderer2D::DrawLine(position, position + line, glm::vec4(1.0f));

						line = forward * spotLight.Range - up * innerRadius * 0.5f;
						Renderer2D::DrawLine(position, position + line, glm::vec4(1.0f));
					}
				}

				Renderer2D::EndScene();
			}
		};

		// { primitive<glm::ivec2>, Attachment, Material, Attachment }
		RenderPass::OnRenderFunc gammaTonemapPass = [](RenderPass::RenderPassContext& context, Ref<Framebuffer> targetFramebuffer, std::vector<IRenderComponent*> inputs) {
			PE_PROFILE_SCOPE("Scene 2D Render Pass");
			Ref<Scene>& sceneContext = context.ActiveScene;
			Ref<Camera> activeCamera = context.ActiveCamera;
			const glm::mat4& cameraWorldTransform = context.CameraWorldTransform;
			PE_CORE_ASSERT(inputs[0], "Viewport resolution input required");
			PE_CORE_ASSERT(inputs[1], "Framebuffer attachment input required");
			PE_CORE_ASSERT(inputs[2], "Material input required");
			PE_CORE_ASSERT(inputs[3], "Source attachment input required");
			RenderComponentPrimitiveType<glm::ivec2>* viewportResInput = dynamic_cast<RenderComponentPrimitiveType<glm::ivec2>*> (inputs[0]);
			RenderComponentFBOAttachment* screenTextureInput = dynamic_cast<RenderComponentFBOAttachment*>(inputs[1]);
			RenderComponentMaterial* gammaCorrectionMaterialInput = dynamic_cast<RenderComponentMaterial*>(inputs[2]);
			RenderComponentFBOAttachment* sourceAttachmentInput = dynamic_cast<RenderComponentFBOAttachment*>(inputs[3]);

			FramebufferTexture2DAttachment* currentAttachment = dynamic_cast<FramebufferTexture2DAttachment*>(targetFramebuffer->GetAttachment(FramebufferAttachmentPoint::Colour0).get());
			FramebufferTexture2DAttachment* targetAttachment = dynamic_cast<FramebufferTexture2DAttachment*>(screenTextureInput->Attachment.get());
			if (currentAttachment->GetTexture() != targetAttachment->GetTexture())
			{
				targetFramebuffer->AddColourAttachment(screenTextureInput->Attachment);
			}
			
			targetFramebuffer->SetDrawBuffers({ FramebufferAttachmentPoint::Colour0 });
			RenderCommand::SetViewport({ 0.0f, 0.0f }, viewportResInput->Data);
			RenderCommand::SetClearColour(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
			RenderCommand::Clear();

			Renderer::BeginScene(activeCamera->GetProjection(), cameraWorldTransform, activeCamera->GetGamma(), activeCamera->GetExposure());
			dynamic_cast<FramebufferTexture2DAttachment*>(sourceAttachmentInput->Attachment.get())->GetTexture()->Bind();
			Renderer::DrawDefaultQuadImmediate(gammaCorrectionMaterialInput->Material, glm::mat4(1.0f), { DepthFunc::ALWAYS, true, true }, FaceCulling::BACK, false, -1);
			Renderer::EndScene();
		};

		// Add render passes
		// -----------------

		// Shadow mapping
		out_Framerenderer.AddRenderPass(RenderPass({ RenderComponentType::PrimitiveType, RenderComponentType::Material }, dirLightShadowPassFunc), dirLightShadowsFramebuffer, { "ShadowResolution", "ShadowmapMaterial" });
		out_Framerenderer.AddRenderPass(RenderPass({ RenderComponentType::PrimitiveType, RenderComponentType::Material }, spotLightShadowPassFunc), spotLightShadowsFramebuffer, { "ShadowResolution", "ShadowmapMaterial" });
		out_Framerenderer.AddRenderPass(RenderPass({ RenderComponentType::PrimitiveType, RenderComponentType::Material, RenderComponentType::UBO }, pointLightShadowFunc), pointLightShadowsFramebuffer, { "ShadowResolution", "ShadowmapCubeMaterial", "CubemapDataUBO" });
	
		// Main render
		out_Framerenderer.AddRenderPass(RenderPass({ RenderComponentType::PrimitiveType, RenderComponentType::FramebufferAttachment }, scene2DPass), m_MainFramebuffer, { "ViewportResolution", "ScreenAttachment" });
		out_Framerenderer.AddRenderPass(RenderPass({ RenderComponentType::PrimitiveType, RenderComponentType::PrimitiveType, RenderComponentType::Texture, RenderComponentType::Texture , RenderComponentType::Texture, RenderComponentType::FramebufferAttachment }, scene3DPass), m_MainFramebuffer, { "ViewportResolution", "ShadowResolution", "DirLightShadowMap", "SpotLightShadowMap", "PointLightShadowMap", "ScreenAttachment" });
		
		// Bloom
		out_Framerenderer.AddRenderPass(RenderPass({ RenderComponentType::PrimitiveType, RenderComponentType::PrimitiveType, RenderComponentType::Material, RenderComponentType::FramebufferAttachment }, bloomDownsamplePass), bloomFBO, { "ViewportResolution", "BloomMipChain", "MipChainDownsampleMaterial", "ScreenAttachment" });

		// Editor overlay
		out_Framerenderer.AddRenderPass(RenderPass({ RenderComponentType::PrimitiveType, RenderComponentType::PrimitiveType, RenderComponentType::PrimitiveType, RenderComponentType::PrimitiveType, RenderComponentType::FramebufferAttachment }, debugOverlayPass), m_MainFramebuffer, { "ShowColliders", "SelectedEntity", "OutlineThickness", "OutlineColour", "ScreenAttachment" });
		
		// Post process
		out_Framerenderer.AddRenderPass(RenderPass({ RenderComponentType::PrimitiveType, RenderComponentType::FramebufferAttachment, RenderComponentType::Material, RenderComponentType::FramebufferAttachment }, gammaTonemapPass), m_MainFramebuffer, { "ViewportResolution", "AlternateScreenAttachment", "GammaTonemapMaterial", "ScreenAttachment" });
	}

	EditorLayer::EditorLayer() : Layer("EditorLayer"), m_ViewportSize(1280.0f, 720.0f), m_CurrentFilepath(std::string()), m_AtlasCreateWindow(0), m_MaterialCreateWindow(0) {}

	EditorLayer::~EditorLayer() {}

	void EditorLayer::OnAttach()
	{
		PE_PROFILE_FUNCTION();

		FramebufferSpecification spec;
		spec.Width = 1280;
		spec.Height = 720;
		spec.Samples = 1;

		TextureSpecification texSpec;
		texSpec.Width = 1280;
		texSpec.Height = 720;
		texSpec.GenerateMips = false;
		texSpec.Format = ImageFormat::RGBA8;
		texSpec.MinFilter = ImageMinFilter::NEAREST;
		texSpec.MagFilter = ImageMagFilter::NEAREST;
		texSpec.Wrap_S = ImageWrap::CLAMP_TO_BORDER;
		texSpec.Wrap_T = ImageWrap::CLAMP_TO_BORDER;
		texSpec.Wrap_R = ImageWrap::CLAMP_TO_BORDER;
		texSpec.Border = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		//Ref<FramebufferTexture2DAttachment> colour0Attach = FramebufferTexture2DAttachment::Create(FramebufferAttachmentPoint::Colour0, texSpec);

		texSpec.Format = ImageFormat::RED_INTEGER;
		Ref<FramebufferTexture2DAttachment> entityIDAttach = FramebufferTexture2DAttachment::Create(FramebufferAttachmentPoint::Colour1, texSpec);

		texSpec.Format = ImageFormat::Depth24Stencil8;
		Ref<FramebufferTexture2DAttachment> depthAttach = FramebufferTexture2DAttachment::Create(FramebufferAttachmentPoint::DepthStencil, texSpec);

		m_MainFramebuffer = Framebuffer::Create(spec, { entityIDAttach }, depthAttach);

		m_EditorScene = CreateRef<Scene>();
		m_ActiveScene = m_EditorScene;
		m_ActiveSceneHandle = 0;

		m_IconPlay = TextureImporter::LoadTexture2D("Resources/Icons/mingcute--play-fill-light.png");
		m_IconStop = TextureImporter::LoadTexture2D("Resources/Icons/mingcute--stop-fill-light.png");
		m_IconSimulate = TextureImporter::LoadTexture2D("Resources/Icons/mingcute--play-line-light.png");

		auto commandLineArgs = Application::Get().GetSpecification().CommandLineArgs;
		if (commandLineArgs.Count > 1) {
			auto projectFilepath = commandLineArgs[1];
			m_ProjectSelected = OpenProject(projectFilepath);
		}

#if 0
		m_SquareEntity = m_ActiveScene->CreateEntity("Square");
		m_SquareEntity.HasComponent<ComponentTransform>();
		m_SquareEntity.AddComponent<Component2DSprite>(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));

		m_SquareTwo = m_ActiveScene->CreateEntity("Square 2");
		m_SquareTwo.AddComponent<Component2DSprite>(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
		m_SquareTwo.GetComponent<ComponentTransform>().Position -= glm::vec3(1.5f, 0.0f, 0.0f);

		m_CameraEntity = m_ActiveScene->CreateEntity("Camera Entity");
		m_CameraEntity.GetComponent<ComponentTransform>().Position = glm::vec3(0.0f, 0.0f, -0.5f);
		m_CameraEntity.AddComponent<ComponentCamera>(SCENE_CAMERA_ORTHOGRAPHIC);

		class CameraController : public EntityScript {
		public:
			void OnCreate() override {
				PE_INFO("Camera Controller Created!");
			}

			void OnDestroy() override {
				PE_INFO("Camera Controller Destroyed");
			}

			void OnUpdate(Timestep timestep) override {
				ComponentTransform& transform = m_Entity.GetComponent<ComponentTransform>();
				float speed = 5.0f;

				if (Input::IsKeyPressed(PE_KEY_W)) {
					transform.Position.y += speed * timestep;
				}
				if (Input::IsKeyPressed(PE_KEY_S)) {
					transform.Position.y -= speed * timestep;
				}
				if (Input::IsKeyPressed(PE_KEY_A)) {
					transform.Position.x -= speed * timestep;
				}
				if (Input::IsKeyPressed(PE_KEY_D)) {
					transform.Position.x += speed * timestep;
				}
			}
		};

		m_CameraEntity.AddComponent<ComponentNativeScript>().Bind<CameraController>();
#endif

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);

		m_Camera = CreateRef<EditorCamera>(EditorCamera(90.0f, 1.778f, 0.01f, 1000.0f));

		// load test shader
		//Ref<EditorAssetManager> assetManager = Project::GetActive()->GetEditorAssetManager();
		//std::filesystem::path engineAssetsRelativeToProjectAssets = std::filesystem::path("assets").lexically_relative(Project::GetAssetDirectory());
		//
		//AssetHandle downsampleTest = assetManager->ImportAsset(engineAssetsRelativeToProjectAssets / "shaders/MipChainDownsample.glsl", true);
		//AssetHandle upsampleTest = assetManager->ImportAsset(engineAssetsRelativeToProjectAssets / "shaders/MipChainUpsample.glsl", true);
		//AssetHandle combineTest = assetManager->ImportAsset(engineAssetsRelativeToProjectAssets / "shaders/MipChainBloomCombine.glsl", true);

		m_Renderer = FrameRenderer();

		if (m_ProjectSelected) {
			CreateRenderer(m_Renderer);
		}
	}

	void EditorLayer::OnDetach()
	{
		PE_PROFILE_FUNCTION();
	}

	static Timestep deltaTime = 0.0f;
	void EditorLayer::OnUpdate(const Timestep timestep)
	{
		PE_PROFILE_FUNCTION();
		deltaTime = timestep;

		// Resize
		const FramebufferSpecification& spec = m_MainFramebuffer->GetSpecification();
		if ((uint32_t)m_ViewportSize.x != spec.Width || (uint32_t)m_ViewportSize.y != spec.Height) {
			OnEvent(MainViewportResizeEvent(m_ViewportSize.x, m_ViewportSize.y));
		}

		Renderer2D::ResetStats();
		Renderer::ResetStats();
		m_MainFramebuffer->Bind();
		RenderCommand::SetViewport({ 0.0f, 0.0f }, glm::ivec2((glm::ivec2)m_ViewportSize));
		RenderCommand::SetClearColour(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
		RenderCommand::Clear();

		// Clear entity ID attachment to -1
		FramebufferTexture2DAttachment* texAttachment = dynamic_cast<FramebufferTexture2DAttachment*>(m_MainFramebuffer->GetAttachment(FramebufferAttachmentPoint::Colour1).get());
		texAttachment->GetTexture()->Clear(-1);

		auto renderResource = m_Renderer.GetRenderResource<RenderComponentPrimitiveType<Entity>>("SelectedEntity");
		if (renderResource) { renderResource->Data = m_SceneHierarchyPanel.GetSelectedEntity(); }

		if (m_ProjectSelected) {
			switch (m_SceneState)
			{
				case SceneState::Edit:
					m_Camera->OnUpdate(timestep, ImGuizmo::IsOver());
					m_Renderer.RenderFrame(m_ActiveScene, m_Camera, glm::inverse(m_Camera->GetViewMatrix()));
					break;
				case SceneState::Simulate:
					m_Camera->OnUpdate(timestep, ImGuizmo::IsOver());
					m_ActiveScene->OnUpdateSimulation(timestep, *m_Camera.get());
					m_Renderer.RenderFrame(m_ActiveScene, m_Camera, glm::inverse(m_Camera->GetViewMatrix()));
					break;
				case SceneState::Play:
					m_ActiveScene->OnUpdateRuntime(timestep);

					Entity cameraEntity = m_ActiveScene->GetPrimaryCameraEntity();
					if (cameraEntity) {
						Ref<SceneCamera> camera = CreateRef<SceneCamera>(cameraEntity.GetComponent<ComponentCamera>().Camera);
						glm::mat4 transform = cameraEntity.GetComponent<ComponentTransform>().GetTransform();
						m_Renderer.RenderFrame(m_ActiveScene, camera, transform);
					}
					else { m_Renderer.RenderFrame(m_ActiveScene, CreateRef<SceneCamera>(SCENE_CAMERA_PERSPECTIVE), glm::mat4(1.0f)); }

					break;
			}

			ImVec2 mousePos = ImGui::GetMousePos();
			mousePos.x -= m_ViewportBounds[0].x;
			mousePos.y -= m_ViewportBounds[0].y;
			glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
			mousePos.y = viewportSize.y - mousePos.y;

			int mouseX = (int)mousePos.x;
			int mouseY = (int)mousePos.y;

			if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y)
			{
				m_HoveredEntity = Entity((entt::entity)m_MainFramebuffer->ReadPixel(FramebufferAttachmentPoint::Colour1, mouseX, mouseY), m_ActiveScene.get());
			}
			else {
				m_HoveredEntity = Entity();
			}
		}

		m_MainFramebuffer->Unbind();
	}

	void EditorLayer::OnImGuiRender()
	{
		PE_PROFILE_FUNCTION();

		static bool dockSpaceOpen = true;
		static bool opt_fullscreen_persistant = true;
		bool opt_fullscreen = opt_fullscreen_persistant;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen) {
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) {
			window_flags |= ImGuiWindowFlags_NoBackground;
		}

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Dockspace", &dockSpaceOpen, window_flags);
		ImGui::PopStyleVar();

		if (opt_fullscreen) {
			ImGui::PopStyleVar(2);
		}

		if (m_ProjectSelected)
		{
			// Dockspace
			ImGuiIO& io = ImGui::GetIO();
			ImGuiStyle& style = ImGui::GetStyle();
			float minWindowSizeX = style.WindowMinSize.x;
			style.WindowMinSize.x = 380.0f;
			if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
				ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
				ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
			}
			style.WindowMinSize.x = minWindowSizeX;

			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("File"))
				{
					ImGui::SeparatorText("Scene");
					if (ImGui::MenuItem("New", "LCtrl+N")) {
						NewScene();
					}
					if (ImGui::MenuItem("Open..", "LCrtl+O"))
					{
						OpenScene();
					}
					ImGui::Separator();
					if (ImGui::MenuItem("Save", "LCtrl+S")) {
						SaveSceneAs(m_CurrentFilepath);
					}
					if (ImGui::MenuItem("Save As..", "LCtrl+LShift+S")) {
						SaveSceneAs();
					}
					ImGui::SeparatorText("Project");
					if (ImGui::MenuItem("New Project..")) { NewProject(); }
					if (ImGui::MenuItem("Open Project..")) { OpenProject(); }
					if (ImGui::MenuItem("Save Project As..")) { SaveProjectAs(); }
					if (ImGui::MenuItem("Exit", "ESC")) { Application::Get().Close(); }
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Edit")) {
					if (ImGui::Checkbox("Show colliders", &m_ShowColliders)) {
						auto resource = m_Renderer.GetRenderResource<RenderComponentPrimitiveType<bool>>("ShowColliders");
						if (resource) { resource->Data = m_ShowColliders; }
					}
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Asset")) {
					if (ImGui::BeginMenu("Create..")) {
						if (ImGui::MenuItem("Material..")) {
							m_MaterialCreateWindow.SetShowWindow(true);
							m_MaterialCreateWindow.SetContext(0);
						}
						if (ImGui::MenuItem("Texture array..")) {
							m_TextureArrayCreateWindow.SetShowWindow(true);
						}
						if (ImGui::MenuItem("Texture Atlas..")) {
							m_AtlasCreateWindow.SetShowWindow(true);
							m_AtlasCreateWindow.SetContext(0);
						}
						ImGui::EndMenu();
					}
					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}

			ImGui::Begin("Renderer Debug");

			std::string hoveredEntityName = "null";
			if (m_HoveredEntity.BelongsToScene(m_ActiveScene) && m_HoveredEntity) {
				hoveredEntityName = m_HoveredEntity.GetComponent<ComponentTag>().Tag;
			}
			ImGui::Text("Hovered entity: %s", hoveredEntityName.c_str());

			ImGui::SeparatorText("Editor Camera:");
			ImGui::DragFloat("Gamma", &m_Camera->Gamma, 0.01f, 0.0f, 100.0f);
			ImGui::DragFloat("Exposure", &m_Camera->Exposure, 0.01f, 0.0f, 100.0f);
			ImGui::Spacing();

			const Renderer::Statistics& stats = Renderer::GetStats();
			ImGui::SeparatorText("Renderer3D Stats:");
			ImGui::Text("Draw Calls: %d", stats.DrawCalls);
			ImGui::Text("Mesh Count: %d", stats.MeshCount);
			ImGui::Text("Pipeline Count: %d", stats.PipelineCount);

			ImGui::Spacing();

			const Renderer2D::Statistics& statsTwo = Renderer2D::GetStats();
			ImGui::SeparatorText("Renderer2D Stats:");
			ImGui::Text("Draw Calls: %d", statsTwo.DrawCalls);
			ImGui::Text("Quad Count: %d", statsTwo.QuadCount);
			ImGui::Text("Vertices: %d", statsTwo.GetTotalVertexCount());
			ImGui::Text("Indices: %d", statsTwo.GetTotalIndexCount());

			ImGui::Spacing();

			ImGui::Text("Timestep (ms): %f", deltaTime.GetMilliseconds());
			ImGui::Text("FPS: %d", (int)(1.0f / deltaTime.GetSeconds()));
			ImGui::End();

			ImGui::Begin("Asset Manager Debug");
			const AssetMap& tempAssets = Project::GetActive()->GetEditorAssetManager()->GetTempAssetMap();
			const AssetMap& persistentAssets = Project::GetActive()->GetEditorAssetManager()->GetPersistentAssetMap();

			ImGui::Text("Temp Assets: %d", tempAssets.size());
			ImGui::Separator();
			for (auto& [handle, asset] : tempAssets) {
				ImGui::BulletText("%d : %s", handle, AssetTypeToString(asset->GetType()).c_str());
			}

			ImGui::Text("Persistent Assets: %d", persistentAssets.size());
			ImGui::Separator();
			for (auto& [handle, asset] : persistentAssets) {
				ImGui::BulletText("%d : %s", handle, AssetTypeToString(asset->GetType()).c_str());
			}

			ImGui::End();

			// -- Viewport --
			// --------------
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
			ImGui::Begin("Viewport");

			ImVec2 viewportMinRegion = ImGui::GetWindowContentRegionMin();
			ImVec2 viewportMaxRegion = ImGui::GetWindowContentRegionMax();
			ImVec2 viewportOffset = ImGui::GetWindowPos();
			m_ViewportBounds[0] = glm::vec2(viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y);
			m_ViewportBounds[1] = glm::vec2(viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y);

			m_ViewportFocus = ImGui::IsWindowFocused();
			m_ViewportHovered = ImGui::IsWindowHovered();
			Application::Get().GetImGuiLayer()->SetBlockEvents(!m_ViewportFocus && !m_ViewportHovered);

			ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
			m_ViewportSize = glm::vec2(viewportPanelSize.x, viewportPanelSize.y);

			FramebufferTexture2DAttachment* texAttachment = dynamic_cast<FramebufferTexture2DAttachment*>(m_MainFramebuffer->GetAttachment(FramebufferAttachmentPoint::Colour0).get());
			uint32_t textureID = texAttachment->GetTexture()->GetRendererID();
			ImGui::Image(textureID, ImVec2(m_ViewportSize.x, m_ViewportSize.y), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

			if (ImGui::BeginDragDropTarget()) {
				const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM");
				if (payload) {
					AssetHandle handle = *(AssetHandle*)payload->Data;
					OpenScene(handle);
				}
				ImGui::EndDragDropTarget();
			}

			// Gizmos
			Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
			if (selectedEntity && m_GizmoType != -1) {
				ImGuizmo::SetOrthographic(false);
				ImGuizmo::SetDrawlist();
				ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y, m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y);

				glm::mat4 cameraView;
				glm::mat4 cameraProjection;

				if (m_SceneState == SceneState::Play) {
					Entity runtimeCameraEntity = m_ActiveScene->GetPrimaryCameraEntity();
					Camera& camera = runtimeCameraEntity.GetComponent<ComponentCamera>().Camera;
					cameraView = glm::inverse(runtimeCameraEntity.GetComponent<ComponentTransform>().GetTransform());
					cameraProjection = camera.GetProjection();
				}
				else {
					cameraView = m_Camera->GetViewMatrix();
					cameraProjection = m_Camera->GetProjection();
				}

				// Selected entity
				ComponentTransform& transformComponent = selectedEntity.GetComponent<ComponentTransform>();
				glm::mat4 entityTransform = transformComponent.GetTransform();

				// Snapping
				bool snap = Input::IsKeyPressed(PE_KEY_LEFT_CONTROL);
				float snapValue = 0.25f;
				if (m_GizmoType == ImGuizmo::OPERATION::ROTATE) {
					snapValue = 45.0f;
				}

				float snapValues[3] = { snapValue, snapValue, snapValue };
				ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection), (ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::MODE::LOCAL, glm::value_ptr(entityTransform),
					nullptr, snap ? snapValues : nullptr);

				if (ImGuizmo::IsUsing()) {
					glm::vec3 position = glm::vec3();
					glm::vec3 rotation = glm::vec3();
					glm::vec3 scale = glm::vec3();
					Maths::DecomposeTransform(entityTransform, position, rotation, scale);
					glm::vec3 deltaRotation = rotation - transformComponent.Rotation();

					transformComponent.SetPosition(position);
					transformComponent.SetRotation(transformComponent.Rotation() + deltaRotation);
					transformComponent.SetScale(scale);
				}
			}

			ImGui::End();
			ImGui::PopStyleVar();

			m_SceneHierarchyPanel.OnImGuiRender();
			m_ContentBrowserPanel->ImGuiRender();
			m_TextureArrayCreateWindow.OnImGuiRender();
			m_MaterialCreateWindow.OnImGuiRender();
			m_AtlasCreateWindow.OnImGuiRender();

			DrawMaterialEdit(AssetManager::GetAsset<Material>(Project::GetActive()->GetEditorAssetManager()->ImportAsset("materials/TestMaterial.pmat", true)));
			DrawMaterialEdit(AssetManager::GetAsset<Material>(Project::GetActive()->GetEditorAssetManager()->ImportAsset("materials/TextureArrayMaterial.pmat", false)));

			OnUIDrawToolbar();
		}
		else 
		{
			DrawProjectSelectUI();
		}
		ImGui::End();
	}

	void EditorLayer::OnUIDrawToolbar()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		auto& colours = ImGui::GetStyle().Colors;
		auto& hovered = colours[ImGuiCol_ButtonHovered];
		auto& active = colours[ImGuiCol_ButtonActive];
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(hovered.x, hovered.y, hovered.z, 0.5f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(active.x, active.y, active.z, 0.5f));

		ImGui::Begin("##Toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

		float size = ImGui::GetContentRegionAvail().y - 4.0f;
		{
			const char* tooltip = (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Simulate) ? "Play" : "Stop";
			Ref<Texture2D> icon = (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Simulate) ? m_IconPlay : m_IconStop;
			ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));
			if (ImGui::ImageButton("##switch_state", icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1))) {
				if (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Simulate) {
					OnScenePlay();
				}
				else if (m_SceneState == SceneState::Play) {
					OnSceneStop();
				}
			}
			ImGui::SetItemTooltip(tooltip);
		}
		ImGui::SameLine();
		{
			const char* tooltip = (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Play) ? "Simulate" : "Stop";
			Ref<Texture2D> icon = (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Play) ? m_IconSimulate : m_IconStop;
			if (ImGui::ImageButton("##switch_state2", icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1))) {
				if (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Play) {
					OnSceneSimulate();
				}
				else if (m_SceneState == SceneState::Simulate) {
					OnSceneStop();
				}
			}
			ImGui::SetItemTooltip(tooltip);
		}

		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(3);
		ImGui::End();
	}

	void EditorLayer::OnScenePlay()
	{
		if (m_SceneState == SceneState::Simulate) {
			OnSceneStop();
		}

		m_ActiveScene = Scene::Copy(m_EditorScene);
		m_ActiveScene->OnRuntimeStart();
		m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
		m_SceneState = SceneState::Play;
	}

	void EditorLayer::OnSceneSimulate()
	{
		if (m_SceneState == SceneState::Play) {
			OnSceneStop();
		}

		m_ActiveScene = Scene::Copy(m_EditorScene);
		m_ActiveScene->OnSimulationStart();

		m_SceneState = SceneState::Simulate;
	}

	void EditorLayer::OnSceneStop()
	{
		if (m_SceneState == SceneState::Play) {
			m_ActiveScene->OnRuntimeStop();
		}
		else if (m_SceneState == SceneState::Simulate) {
			m_ActiveScene->OnSimulationStop();
		}
		else {
			return;
		}

		m_ActiveScene = m_EditorScene;
		m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
		m_SceneState = SceneState::Edit;
	}

	void EditorLayer::OnDuplicatedEntity()
	{
		if (m_SceneState == SceneState::Edit) {
			Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
			if (selectedEntity) {
				Entity newEntity = m_ActiveScene->DuplicateEntity(selectedEntity);
				m_SceneHierarchyPanel.SetSelectedEntity(newEntity);
			}
		}
	}

	void EditorLayer::OnDebugOverlayDraw()
	{
		PE_PROFILE_FUNCTION();
		if (m_SceneState == SceneState::Play) {
			Entity cameraEntity = m_ActiveScene->GetPrimaryCameraEntity();
			if (!cameraEntity) { return; }
			Renderer2D::BeginScene(cameraEntity.GetComponent<ComponentCamera>().Camera, cameraEntity.GetComponent<ComponentTransform>().GetTransform(), FaceCulling::NONE, { DepthFunc::ALWAYS, true, true });
		}
		else {
			Renderer2D::BeginScene(*m_Camera.get(), FaceCulling::NONE, { DepthFunc::ALWAYS, true, true });
		}

		if (m_ShowColliders) {
			// Box colliders
			auto boxView = m_ActiveScene->View<ComponentTransform, ComponentBoxCollider2D>();
			for (auto entityID : boxView) {
				auto [transform, box] = boxView.get<ComponentTransform, ComponentBoxCollider2D>(entityID);

				glm::vec3 position = glm::vec3(glm::vec2(transform.Position()), 0.01f);
				glm::vec3 scale = transform.Scale() * (glm::vec3(box.Size() * 2.0f, 1.0f));
				glm::mat4 transformation = glm::translate(glm::mat4(1.0f), position);
				transformation = glm::rotate(transformation, transform.Rotation().z, glm::vec3(0.0, 0.0, 1.0f));
				transformation = glm::scale(transformation, scale);

				Renderer2D::SetLineWidth(0.01f);
				Renderer2D::DrawRect(transformation, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), (int)entityID);
			}
			
			// Circle colliders
			auto circleView = m_ActiveScene->View<ComponentTransform, ComponentCircleCollider2D>();
			for (auto entityID : circleView) {
				auto [transform, circle] = circleView.get<ComponentTransform, ComponentCircleCollider2D>(entityID);

				glm::vec3 position = glm::vec3(glm::vec2(transform.Position()), 0.01f);
				glm::vec3 scale = transform.Scale() * (circle.Radius() * 2.0f);
				glm::mat4 transformation = glm::translate(glm::mat4(1.0f), position);
				transformation = glm::scale(transformation, scale);

				Renderer2D::DrawCircle(transformation, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), 0.01f, 0.0f, (int)entityID);
			}
		}
		Renderer2D::Flush();

		Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
		if (selectedEntity.BelongsToScene(m_ActiveScene) && selectedEntity) {
			// Entity outline
			ComponentTransform transformCopy = selectedEntity.GetComponent<ComponentTransform>();
			transformCopy.SetPosition(transformCopy.Position() + glm::vec3(0.0f, 0.0f, 0.01f));
			Renderer2D::SetLineWidth(m_EntityOutlineThickness);
			Renderer2D::DrawRect(transformCopy.GetTransform(), m_EntityOutlineColour);
		
			// Point light radius
			if (selectedEntity.HasComponent<ComponentPointLight>()) {
				ComponentTransform& transformComponent = selectedEntity.GetComponent<ComponentTransform>();
				ComponentPointLight& pointLight = selectedEntity.GetComponent<ComponentPointLight>();
				float radius = pointLight.Radius;
				float thickness = 0.005f;
				float fade = 0.0f;

				glm::mat4 transform = glm::mat4(1.0f);
				transform = glm::translate(transform, transformComponent.Position());
				transform = glm::scale(transform, glm::vec3(radius, radius, 1.0f));
				Renderer2D::DrawCircle(transform, glm::vec4(1.0f), thickness, fade);

				transform = glm::mat4(1.0f);
				transform = glm::translate(transform, transformComponent.Position());
				transform = glm::rotate(transform, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
				transform = glm::scale(transform, glm::vec3(radius, radius, 1.0f));
				Renderer2D::DrawCircle(transform, glm::vec4(1.0f), thickness, fade);

				transform = glm::mat4(1.0f);
				transform = glm::translate(transform, transformComponent.Position());
				transform = glm::rotate(transform, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				transform = glm::scale(transform, glm::vec3(radius, radius, 1.0f));
				Renderer2D::DrawCircle(transform, glm::vec4(1.0f), thickness, fade);
			}

			// Spot light
			if (selectedEntity.HasComponent<ComponentSpotLight>()) {
				ComponentTransform& transformComponent = selectedEntity.GetComponent<ComponentTransform>();
				ComponentSpotLight& spotLight = selectedEntity.GetComponent<ComponentSpotLight>();
				glm::vec3 position = transformComponent.Position();
				glm::quat rotationQuat = glm::quat(transformComponent.Rotation());
				float outerRadius = glm::tan(glm::radians(spotLight.OuterCutoff)) * spotLight.Range;
				float innerRadius = glm::tan(glm::radians(spotLight.InnerCutoff)) * spotLight.Range;
				float thickness = 0.005f;
				float fade = 0.0f;

				glm::mat4 transform = glm::mat4(1.0f);
				transform = glm::translate(transform, position);
				glm::mat4 rotation = glm::toMat4(rotationQuat);
				transform *= rotation;
				transform = glm::translate(transform, glm::vec3(0.0, 0.0f, -spotLight.Range));

				glm::vec3 forward = glm::normalize(glm::rotate(rotationQuat, glm::vec3(0.0f, 0.0f, -1.0f)));
				glm::vec3 right = glm::normalize(glm::rotate(rotationQuat, glm::vec3(1.0f, 0.0f, 0.0f)));
				glm::vec3 up = glm::cross(forward, right);

				Renderer2D::SetLineWidth(thickness);

				// Outer cone
				glm::mat4 outerCircleTransform = glm::scale(transform, glm::vec3(outerRadius, outerRadius, 1.0f));
				Renderer2D::DrawCircle(outerCircleTransform, glm::vec4(1.0f), thickness, fade);

				glm::vec3 line = forward * spotLight.Range + right * outerRadius * 0.5f;
				Renderer2D::DrawLine(position, position + line, glm::vec4(1.0f));

				line = forward * spotLight.Range - right * outerRadius * 0.5f;
				Renderer2D::DrawLine(position, position + line, glm::vec4(1.0f));

				line = forward * spotLight.Range + up * outerRadius * 0.5f;
				Renderer2D::DrawLine(position, position + line, glm::vec4(1.0f));

				line = forward * spotLight.Range - up * outerRadius * 0.5f;
				Renderer2D::DrawLine(position, position + line, glm::vec4(1.0f));

				// Inner cone
				glm::mat4 innerCircleTransform = glm::scale(transform, glm::vec3(innerRadius, innerRadius, 1.0f));
				Renderer2D::DrawCircle(innerCircleTransform, glm::vec4(1.0f), thickness, fade);

				line = forward * spotLight.Range + right * innerRadius * 0.5f;
				Renderer2D::DrawLine(position, position + line, glm::vec4(1.0f));

				line = forward * spotLight.Range - right * innerRadius * 0.5f;
				Renderer2D::DrawLine(position, position + line, glm::vec4(1.0f));

				line = forward * spotLight.Range + up * innerRadius * 0.5f;
				Renderer2D::DrawLine(position, position + line, glm::vec4(1.0f));

				line = forward * spotLight.Range - up * innerRadius * 0.5f;
				Renderer2D::DrawLine(position, position + line, glm::vec4(1.0f));
			}
		}

		Renderer2D::EndScene();
	}

	void EditorLayer::OnEvent(Event& e)
	{
		PE_PROFILE_FUNCTION();

		EventDispatcher dispatcher = EventDispatcher(e);
		dispatcher.DispatchEvent<MainViewportResizeEvent>(PE_BIND_EVENT_FN(EditorLayer::OnViewportResize));
		dispatcher.DispatchEvent<KeyReleasedEvent>(PE_BIND_EVENT_FN(EditorLayer::OnKeyUp));
		dispatcher.DispatchEvent<MouseButtonReleasedEvent>(PE_BIND_EVENT_FN(EditorLayer::OnMouseUp));
		dispatcher.DispatchEvent<WindowDropEvent>(PE_BIND_EVENT_FN(EditorLayer::OnWindowDrop));
		dispatcher.DispatchEvent<SceneChangedEvent>(PE_BIND_EVENT_FN(EditorLayer::OnSceneChanged));

		if (m_SceneState != SceneState::Play) {
			m_Camera->OnEvent(e);
		}

		m_Renderer.OnEvent(e);
	}

	bool EditorLayer::OnKeyUp(KeyReleasedEvent& e)
	{
		bool LControl = Input::IsKeyPressed(PE_KEY_LEFT_CONTROL);
		bool LShift = Input::IsKeyPressed(PE_KEY_LEFT_SHIFT);
		switch (e.GetKeyCode()) {

			// File shortcuts
			case PE_KEY_ESCAPE:
				Application::Get().Close();
				return true;
				break;
			case PE_KEY_DELETE:
				{
					Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
					if (selectedEntity.BelongsToScene(m_ActiveScene) && selectedEntity) {
						m_SceneHierarchyPanel.SetSelectedEntity(Entity());
						m_ActiveScene->DestroyEntity(selectedEntity);
					}
					return true;
					break;
				}
			case PE_KEY_N:
				if (LControl)
				{
					NewScene();
					return true;
				}
				break;
			case PE_KEY_O:
				if (LControl)
				{
					OpenScene();
					return true;
				}
				break;
			case PE_KEY_S:
				if (LControl)
				{
					if (LShift) { SaveSceneAs(); }
					else { SaveSceneAs(m_CurrentFilepath); }
				}
				return true;
				break;

			// Gizmo shortcuts
			case PE_KEY_Q:
				m_GizmoType = -1;
				return true;
				break;
			case PE_KEY_W:
				m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
				return true;
				break;
			case PE_KEY_E:
				m_GizmoType = ImGuizmo::OPERATION::ROTATE;
				return true;
				break;
			case PE_KEY_R:
				m_GizmoType = ImGuizmo::OPERATION::SCALE;
				return true;
				break;

			// Entity shortcuts
			case PE_KEY_D:
				if (LControl) {
					OnDuplicatedEntity();
				}
				return true;
				break;
		}
		return false;
	}

	bool EditorLayer::OnMouseUp(MouseButtonReleasedEvent& e)
	{
		if (e.GetMouseButton() == PE_MOUSE_BUTTON_LEFT && CanPickEntities()) {
			m_SceneHierarchyPanel.SetSelectedEntity(m_HoveredEntity);
			return true;
		}
		return false;
	}

	bool EditorLayer::OnWindowDrop(WindowDropEvent& e)
	{
		for (const std::filesystem::path& path : e.GetPaths()) {
			PE_CORE_INFO("File dropped in window with path: '{0}'", path.string().c_str());
		}

		// TODO: ImportAsset();
		return true;
	}

	bool EditorLayer::OnSceneChanged(SceneChangedEvent& e)
	{
		PE_CORE_INFO(e);
		return true;
	}

	bool EditorLayer::OnViewportResize(MainViewportResizeEvent& e)
	{
		m_MainFramebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);

		m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		m_Camera->SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
		return false;
	}

	void EditorLayer::NewScene()
	{
		OnSceneStop();
		m_EditorScene = CreateRef<Scene>();
		m_ActiveScene = m_EditorScene;
		m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
		m_CurrentFilepath = std::filesystem::path();
		m_ActiveSceneHandle = m_ActiveScene->Handle;
	}

	void EditorLayer::OpenScene()
	{
		//std::string filepath = FileDialogs::OpenFile("Paul Engine Scene (*.paul)\0*.paul\0");
		//
		//if (!filepath.empty())
		//{
		//	OpenScene(filepath);
		//}
	}

	void EditorLayer::OpenScene(AssetHandle handle)
	{
		PE_CORE_ASSERT(handle, "Invalid scene handle");
		Ref<EditorAssetManager> assetManager = Project::GetActive()->GetEditorAssetManager();
		AssetType type = assetManager->GetAssetType(handle);
		if (type != AssetType::Scene) {
			PE_CORE_ERROR("Invalid asset type '{0}', '{1}' required", AssetTypeToString(type), AssetTypeToString(AssetType::Scene));
			return;
		}
		if (m_SceneState != SceneState::Edit) { OnSceneStop(); }

		Ref<Scene> readOnlyScene = AssetManager::GetAsset<Scene>(handle);
		Ref<Scene> newScene = Scene::Copy(readOnlyScene);

		m_EditorScene = newScene;
		m_EditorScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		m_SceneHierarchyPanel.SetContext(m_EditorScene);

		if (m_ActiveSceneHandle != 0) {
			//Project::GetActive()->GetEditorAssetManager()->UnloadAsset(m_ActiveSceneHandle);
			assetManager->ReleaseTempAssets();
		}

		m_ActiveScene = m_EditorScene;
		m_ActiveSceneHandle = handle;

		m_CurrentFilepath = assetManager->GetFilepath(handle);
		if (!assetManager->IsAssetLoaded(handle)) { assetManager->AddToLoadedAssets(m_ActiveScene, assetManager->GetMetadata(handle).Persistent); }

		Application::Get().OnEvent(SceneChangedEvent(m_ActiveSceneHandle));
	}

	void EditorLayer::SaveSceneAs(const std::filesystem::path& filepath)
	{
		std::string path = filepath.string();
		if (path.empty()) {
			std::filesystem::path absoluteAssetDirPath = std::filesystem::absolute(Project::GetActive()->GetAssetDirectory());
			path = FileDialogs::SaveFile("Paul Engine Scene (*.paul)\0*.paul\0");
			std::filesystem::path filepath = path;
			path = filepath.lexically_relative(absoluteAssetDirPath).string();
		}
		if (!path.empty()) {
			SceneImporter::SaveScene(m_ActiveScene, path);
			m_CurrentFilepath = path;
		}
	}

	bool EditorLayer::NewProject(std::filesystem::path filepath)
	{
		if (filepath.empty())
		{
			filepath = std::filesystem::proximate(FileDialogs::SaveFile("Paul Engine Project (*.pproj)\0*.pproj\0"));
		}
		if (filepath.empty()) { return false; }

		ProjectSpecification spec;
		spec.Name = filepath.filename().string();
		spec.AssetDirectory = "assets";
		spec.AssetRegistryPath = "asset_registry.pregistry";
		spec.ProjectDirectory = filepath;
		
		NewScene();
		spec.StartScene = m_ActiveSceneHandle;
		Project::New(spec);

		SaveSceneAs("Scenes/UntitledScene.paul");

		AssetMetadata sceneData;
		sceneData.FilePath = m_CurrentFilepath;
		sceneData.Persistent = false;
		sceneData.Type = AssetType::Scene;

		Project::GetActive()->GetEditorAssetManager()->RegisterAsset(m_ActiveSceneHandle, sceneData);

		Project::SaveActive(filepath);

		m_ContentBrowserPanel = CreateScope<ContentBrowserPanel>();

		return true;
	}

	bool EditorLayer::OpenProject()
	{
		std::string filepath = FileDialogs::OpenFile("Paul Engine Project (*.pproj)\0*.pproj\0");

		if (!filepath.empty())
		{
			OpenProject(std::filesystem::proximate(filepath));
			return true;
		}
		else {
			return false;
		}
	}

	bool EditorLayer::OpenProject(const std::filesystem::path& path)
	{
		if (Project::Load(path)) {
			AssetHandle startScene = Project::GetActive()->GetSpecification().StartScene;
			if (startScene) {
				OpenScene(startScene);
			}
			m_ContentBrowserPanel = CreateScope<ContentBrowserPanel>();
			return true;
		}
		PE_CORE_ERROR("Failed to open project at path '{0}'", path.string().c_str());
		return false;
	}

	void EditorLayer::SaveProjectAs()
	{
		std::string path = FileDialogs::SaveFile("Paul Engine Project (*.pproj)\0*.pproj\0");
		if (!path.empty()) {
			Project::SaveActive(std::filesystem::proximate(path));
		}
	}

	void EditorLayer::DrawProjectSelectUI()
	{
		static bool open = true;
		ImGui::SetNextWindowSizeConstraints(ImVec2(598.0f, 254.0f), ImVec2(1000.0f, 1000.0f));
		ImGui::Begin("##Paul Engine", &open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::Text("Welcome to Paul Engine.");

		ImVec2 contentAvail = ImGui::GetContentRegionAvail();
		ImVec2 newTextSize = ImGui::CalcTextSize("New Project..");
		ImVec2 openTextSize = ImGui::CalcTextSize("Open Project..");
		ImVec2 newSize = ImVec2(std::max(newTextSize.x, (contentAvail.x / 3.0f)), std::max(newTextSize.y * 2.0f, (contentAvail.y / 4.0f)));
		ImVec2 openSize = ImVec2(std::max(openTextSize.x, (contentAvail.x / 3.0f)), std::max(openTextSize.y * 2.0f, (contentAvail.y / 4.0f)));

		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
		ImGui::SetCursorPos(ImVec2(contentAvail.x * 0.15f, contentAvail.y - newSize.y));
		if (ImGui::Button("New Project..", newSize)) {
			m_ProjectSelected = NewProject();
		}
		ImGui::SameLine();
		ImGui::SetCursorPos(ImVec2(contentAvail.x - (contentAvail.x * 0.15f) - openSize.x, ImGui::GetCursorPosY()));
		if (ImGui::Button("Open Project..", openSize)) {
			m_ProjectSelected = OpenProject();
		}
		ImGui::PopStyleVar();
		ImGui::End();

		if (m_ProjectSelected)
		{
			m_Renderer = FrameRenderer();
			CreateRenderer(m_Renderer);
		}
	}

	bool EditorLayer::CanPickEntities()
	{
		return m_ViewportHovered && !ImGuizmo::IsOver() && !m_Camera->IsMoving();
	}

	// TEMP
	void EditorLayer::DrawMaterialEdit(Ref<Material> material)
	{
		const AssetMetadata& metadata = Project::GetActive()->GetEditorAssetManager()->GetMetadata(material->Handle);
		ImGui::Begin(metadata.FilePath.stem().string().c_str());

		if (ImGui::Button("Save As...")) {
			std::string path = FileDialogs::SaveFile("Paul Engine Material (*.pmat)\0*.pmat\0");
			if (!path.empty()) {
				std::filesystem::path absoluteProjectPath = std::filesystem::absolute(Project::GetProjectDirectory());
				std::filesystem::path relativeSavePath = std::filesystem::path(path).lexically_relative(absoluteProjectPath.parent_path());

				MaterialImporter::SaveMaterial(material, relativeSavePath);
				AssetHandle handle = Project::GetActive()->GetEditorAssetManager()->ImportAsset(relativeSavePath.lexically_relative(Project::GetAssetDirectory()), false);
			}
		}

		for (auto& it : material->m_ShaderParameters) {
			switch (it.second->GetType()) {
				case ShaderParameterType::UBO:
				{
					UBOShaderParameterTypeStorage* ubo = dynamic_cast<UBOShaderParameterTypeStorage*>(it.second.get());

					for (auto& [name, value] : ubo->UBO()->GetLayoutStorage()) {
						switch (value->GetType()) {
							case ShaderDataType::Float4:
							{
								glm::vec4* data = static_cast<glm::vec4*>(value->GetData());
								ImGui::ColorEdit4(name.c_str(), &(*data)[0]);
								break;
							}
							case ShaderDataType::Float3:
							{
								glm::vec3* data = static_cast<glm::vec3*>(value->GetData());
								ImGui::ColorEdit3(name.c_str(), &(*data)[0]);
								break;
							}
							case ShaderDataType::Float2:
							{
								glm::vec2* data = static_cast<glm::vec2*>(value->GetData());
								ImGui::DragFloat2(name.c_str(), &(*data)[0]);
								break;
							}
							case ShaderDataType::Float:
							{
								float* data = static_cast<float*>(value->GetData());
								ImGui::DragFloat(name.c_str(), &(*data), 0.1f, 0.0f, 1.0f);
								break;
							}
							case ShaderDataType::Int:
							{
								int* data = static_cast<int*>(value->GetData());
								ImGui::DragInt(name.c_str(), &(*data), 1.0f, 0, 1000);
								break;
							}
						}
						ImGui::Text(name.c_str());
					}
					break;
				}
				case ShaderParameterType::Sampler2D:
				{
					const std::string& paramName = it.first;
					ImGui::Text(paramName.c_str());
					Sampler2DShaderParameterTypeStorage* sampler2D = dynamic_cast<Sampler2DShaderParameterTypeStorage*>(it.second.get());
					AssetHandle textureHandle = sampler2D->m_TextureHandle;

					std::string label = "None";
					bool isTextureValid = false;
					if (textureHandle != 0) {
						if (AssetManager::IsAssetHandleValid(textureHandle) && AssetManager::GetAssetType(textureHandle) == AssetType::Texture2D) {
							const AssetMetadata& metadata = Project::GetActive()->GetEditorAssetManager()->GetMetadata(textureHandle);
							label = metadata.FilePath.filename().string();
							isTextureValid = true;
						}
						else {
							label = "Invalid";
						}
					}

					ImVec2 buttonLabelSize = ImGui::CalcTextSize(label.c_str());
					buttonLabelSize.x += 20.0f;
					float buttonLabelWidth = glm::max<float>(100.0f, buttonLabelSize.x);

					ImGui::Button(label.c_str(), ImVec2(buttonLabelWidth, 0.0f));
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
						{
							AssetHandle handle = *(AssetHandle*)payload->Data;
							if (AssetManager::GetAssetType(handle) == AssetType::Texture2D) {
								sampler2D->m_TextureHandle = handle;
							}
							else {
								PE_CORE_WARN("Invalid asset type. Texture2D needed for sampler2D shader parameter");
							}
						}
						ImGui::EndDragDropTarget();
					}

					if (isTextureValid) {
						ImGui::SameLine();
						ImVec2 xLabelSize = ImGui::CalcTextSize("X");
						float buttonSize = xLabelSize.y + ImGui::GetStyle().FramePadding.y * 2.0f;
						if (ImGui::Button("X", ImVec2(buttonSize, buttonSize))) {
							sampler2D->m_TextureHandle = 0;
						}
					}
					ImGui::SameLine();
					ImGui::Text("Texture");

					ImGui::Text("Binding: {0}", sampler2D->m_Binding);

					break;
				}
				case ShaderParameterType::Sampler2DArray:
				{
					const std::string& paramName = it.first;
					ImGui::Text(paramName.c_str());
					Sampler2DArrayShaderParameterTypeStorage* sampler2Darray = dynamic_cast<Sampler2DArrayShaderParameterTypeStorage*>(it.second.get());
					AssetHandle textureHandle = sampler2Darray->m_TextureArrayHandle;

					std::string label = "None";
					bool isTextureValid = false;
					if (textureHandle != 0) {
						if (AssetManager::IsAssetHandleValid(textureHandle) && AssetManager::GetAssetType(textureHandle) == AssetType::Texture2DArray) {
							const AssetMetadata& metadata = Project::GetActive()->GetEditorAssetManager()->GetMetadata(textureHandle);
							label = metadata.FilePath.filename().string();
							isTextureValid = true;
						}
						else {
							label = "Invalid";
						}
					}

					ImVec2 buttonLabelSize = ImGui::CalcTextSize(label.c_str());
					buttonLabelSize.x += 20.0f;
					float buttonLabelWidth = glm::max<float>(100.0f, buttonLabelSize.x);

					ImGui::Button(label.c_str(), ImVec2(buttonLabelWidth, 0.0f));
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
						{
							AssetHandle handle = *(AssetHandle*)payload->Data;
							if (AssetManager::GetAssetType(handle) == AssetType::Texture2DArray) {
								sampler2Darray->m_TextureArrayHandle = handle;
							}
							else {
								PE_CORE_WARN("Invalid asset type. Texture2DArray needed for sampler2Darray shader parameter");
							}
						}
						ImGui::EndDragDropTarget();
					}

					if (isTextureValid) {
						ImGui::SameLine();
						ImVec2 xLabelSize = ImGui::CalcTextSize("X");
						float buttonSize = xLabelSize.y + ImGui::GetStyle().FramePadding.y * 2.0f;
						if (ImGui::Button("X", ImVec2(buttonSize, buttonSize))) {
							sampler2Darray->m_TextureArrayHandle = 0;
						}
					}
					ImGui::SameLine();
					ImGui::Text("Texture Array");

					ImGui::Text("Binding: {0}", sampler2Darray->m_Binding);

					break;
				}
			}
		}
		ImGui::End();
	}
}