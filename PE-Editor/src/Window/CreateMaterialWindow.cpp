#include "pepch.h"
#include "CreateMaterialWindow.h"

#include "PaulEngine/Renderer/Renderer.h"
#include "PaulEngine/Renderer/RenderCommand.h"

#include "PaulEngine/Asset/AssetManager.h"
#include "PaulEngine/Asset/MaterialImporter.h"
#include "PaulEngine/Asset/TextureImporter.h"
#include "PaulEngine/Renderer/Asset/Material.h"

#include "PaulEngine/Utils/PlatformUtils.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <glm/glm.hpp>
#include <imgui_internal.h>

namespace PaulEngine
{
	static glm::mat4 s_MeshTransform = glm::mat4(1.0f);
	static glm::vec3 s_LightColour = glm::vec3(1.0f);
	static ImVec4 s_IconTintColourSelected = ImVec4(37.0f / 255.0f, 122.0f / 255.0f, 253.0f / 255.0f, 1.0f);
	static ImVec4 s_IconTintColour = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	static float s_LightIntensity = 1.0f;
	CreateMaterialWindow::CreateMaterialWindow(AssetHandle shaderHandle) : m_ShaderHandle(shaderHandle), m_ViewportSize(128.0f, 128.0f) {}

	void CreateMaterialWindow::CreateRenderer(FrameRenderer& out_Framerenderer)
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
		Ref<Texture2D> screenTexture = AssetManager::CreateAsset<Texture2D>(true, screenSpec);
		Ref<Texture2D> alternateScreenTexture = AssetManager::CreateAsset<Texture2D>(true, screenSpec);
		out_Framerenderer.AddRenderResource<RenderComponentTexture>("ScreenTexture", false, screenTexture->Handle);
		out_Framerenderer.AddRenderResource<RenderComponentTexture>("AlternateScreenTexture", false, alternateScreenTexture->Handle);

		Ref<FramebufferTexture2DAttachment> screenAttachment = FramebufferTexture2DAttachment::Create(FramebufferAttachmentPoint::Colour0, screenTexture->Handle);

		m_Framebuffer->AddColourAttachment(screenAttachment);

		//  Data
		// ------
		glm::ivec2 viewportRes = { (glm::ivec2)m_ViewportSize };
		out_Framerenderer.AddRenderResource<RenderComponentPrimitiveType<glm::ivec2>>("ViewportResolution", false, viewportRes);

		Ref<Material> material = m_Material;
		out_Framerenderer.AddRenderResource<RenderComponentPrimitiveType<Ref<Material>>>("PreviewMaterial", false, material);

		bool sphereSelected = m_SphereSelected;
		out_Framerenderer.AddRenderResource<RenderComponentPrimitiveType<bool>>("SphereSelected", false, sphereSelected);

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

					Ref<Texture2D> texture = AssetManager::CreateAsset<Texture2D>(true, mipSpec);
					m_Chain.push_back(texture->Handle);
				}
			}

			void Resize(const glm::ivec2 viewportSize)
			{
				glm::vec2 mipSize = viewportSize;
				for (uint8_t i = 0; i < m_Chain.size(); i++)
				{
					mipSize *= 0.5f;
					mipSize = glm::max(mipSize, glm::vec2(1.0f, 1.0f));
					AssetManager::GetAsset<Texture2D>(m_Chain[i])->Resize(mipSize.x, mipSize.y);
				}
			}

			size_t Size() const { return m_Chain.size(); }

			Ref<Texture2D> GetMipLevel(uint8_t mip) {
				PE_CORE_ASSERT(mip < m_Chain.size(), "Index out of bounds. Index: {0}, Size: {1}", mip, m_Chain.size());
				return AssetManager::GetAsset<Texture2D>(m_Chain[mip]);
			}

			AssetHandle GetMipHandle(uint8_t mip) {
				PE_CORE_ASSERT(mip < m_Chain.size(), "Index out of bounds. Index: {0}, Size: {1}", mip, m_Chain.size());
				return m_Chain[mip];
			}

		private:
			std::vector<AssetHandle> m_Chain;
		};
		BloomMipChain bloomMipChain;
		bloomMipChain.Init(m_ViewportSize, 6);

		out_Framerenderer.AddRenderResource<RenderComponentPrimitiveType<BloomMipChain>>("BloomMipChain", false, bloomMipChain);

		// Framebuffers
		// ------------
		FramebufferSpecification bloomSpec;
		bloomSpec.Width = m_ViewportSize.x;
		bloomSpec.Height = m_ViewportSize.y;
		Ref<FramebufferTexture2DAttachment> bloomColourAttachment = FramebufferTexture2DAttachment::Create(FramebufferAttachmentPoint::Colour0, bloomMipChain.GetMipLevel(0)->Handle);
		Ref<Framebuffer> bloomFBO = Framebuffer::Create(bloomSpec, { bloomColourAttachment });

		out_Framerenderer.AddRenderResource<RenderComponentFramebuffer>("MainFramebuffer", false, m_Framebuffer);
		out_Framerenderer.AddRenderResource<RenderComponentFramebuffer>("BloomFramebuffer", false, bloomFBO);

		// Materials
		// ---------
		Ref<EditorAssetManager> assetManager = Project::GetActive()->GetEditorAssetManager();
		std::filesystem::path engineAssetsRelativeToProjectAssets = std::filesystem::path("assets").lexically_relative(Project::GetAssetDirectory());

		AssetHandle gammaTonemapShaderHandle = assetManager->ImportAssetFromFile(engineAssetsRelativeToProjectAssets / "shaders/GammaTonemap.glsl", true);
		Ref<Material> gammaTonemapMaterial = AssetManager::CreateAsset<Material>(true, gammaTonemapShaderHandle);

		AssetHandle bloomDownsampleShaderHandle = assetManager->ImportAssetFromFile(engineAssetsRelativeToProjectAssets / "shaders/MipChainDownsample.glsl", true);
		Ref<Material> mipchainDownsampleMaterial = AssetManager::CreateAsset<Material>(true, bloomDownsampleShaderHandle);

		AssetHandle bloomUpsampleShaderHandle = assetManager->ImportAssetFromFile(engineAssetsRelativeToProjectAssets / "shaders/MipChainUpsample.glsl", true);
		Ref<Material> mipchainUpsampleMaterial = AssetManager::CreateAsset<Material>(true, bloomUpsampleShaderHandle);

		AssetHandle bloomCombineShaderHandle = assetManager->ImportAssetFromFile(engineAssetsRelativeToProjectAssets / "shaders/MipChainBloomCombine.glsl", true);
		Ref<Material> bloomCombineMaterial = AssetManager::CreateAsset<Material>(true, bloomCombineShaderHandle);

		AssetHandle skyboxShaderHandle = assetManager->ImportAssetFromFile(engineAssetsRelativeToProjectAssets / "shaders/Skybox.glsl", true);
		Ref<Material> skyboxMaterial = AssetManager::CreateAsset<Material>(true, skyboxShaderHandle);

		out_Framerenderer.AddRenderResource<RenderComponentMaterial>("GammaTonemapMaterial", false, gammaTonemapMaterial->Handle);
		out_Framerenderer.AddRenderResource<RenderComponentMaterial>("MipChainDownsampleMaterial", false, mipchainDownsampleMaterial->Handle);
		out_Framerenderer.AddRenderResource<RenderComponentMaterial>("MipChainUpsampleMaterial", false, mipchainUpsampleMaterial->Handle);
		out_Framerenderer.AddRenderResource<RenderComponentMaterial>("BloomCombineMaterial", false, bloomCombineMaterial->Handle);
		out_Framerenderer.AddRenderResource<RenderComponentMaterial>("SkyboxMaterial", false, skyboxMaterial->Handle);

		// Textures
		// --------
		AssetHandle dirtMaskTextureHandle = assetManager->ImportAssetFromFile(engineAssetsRelativeToProjectAssets / "textures/dirtmask.jpg", true);

		// Create skybox cubemap texture from individual faces
		// TODO: cubemap asset as a single file (probably another custom file format similar to binary texture array file)

		std::vector<std::filesystem::path> facePaths = {
			"assets/textures/cubemap/default_skybox/right.png",
			"assets/textures/cubemap/default_skybox/left.png",
			"assets/textures/cubemap/default_skybox/top.png",
			"assets/textures/cubemap/default_skybox/bottom.png",
			"assets/textures/cubemap/default_skybox/front.png",
			"assets/textures/cubemap/default_skybox/back.png"
		};

		std::vector<Buffer> faceData;
		faceData.reserve(6);
		for (int i = 0; i < 6; i++)
		{
			TextureImporter::ImageFileReadResult result;
			faceData.push_back(TextureImporter::ReadImageFile(facePaths[i], result, false));
		}

		TextureSpecification skyboxSpec;
		skyboxSpec.Format = ImageFormat::RGB8;
		skyboxSpec.MinFilter = ImageMinFilter::LINEAR;
		skyboxSpec.MagFilter = ImageMagFilter::LINEAR;
		skyboxSpec.Wrap_S = ImageWrap::CLAMP_TO_EDGE;
		skyboxSpec.Wrap_T = ImageWrap::CLAMP_TO_EDGE;
		skyboxSpec.Wrap_R = ImageWrap::CLAMP_TO_EDGE;
		skyboxSpec.Width = 2048;
		skyboxSpec.Height = 2048;
		Ref<TextureCubemap> skyboxCubemap = AssetManager::CreateAsset<TextureCubemap>(true, skyboxSpec, faceData);
		skyboxMaterial->GetParameter<SamplerCubeShaderParameterTypeStorage>("Skybox")->TextureHandle = skyboxCubemap->Handle;

		AssetHandle envMapHandle = assetManager->ImportAssetFromFile(engineAssetsRelativeToProjectAssets / "textures/environment/default_environment.hdr", false);
		out_Framerenderer.AddRenderResource<RenderComponentEnvironmentMap>("EnvironmentMap", true, envMapHandle);

		out_Framerenderer.AddRenderResource<RenderComponentTexture>("DirtMaskTexture", true, dirtMaskTextureHandle);
		out_Framerenderer.AddRenderResource<RenderComponentTexture>("SkyboxTexture", true, skyboxCubemap->Handle);

		// OnEvent
		// -------
		FrameRenderer::OnEventFunc eventFunc = [](Event& e, FrameRenderer* self)
		{
			EventDispatcher dispatcher = EventDispatcher(e);
			dispatcher.DispatchEvent<MainViewportResizeEvent>([self](MainViewportResizeEvent& e)->bool {
				glm::ivec2 viewportSize = glm::ivec2(e.GetWidth(), e.GetHeight());
				self->GetRenderResource<RenderComponentPrimitiveType<glm::ivec2>>("ViewportResolution")->Data = viewportSize;
				AssetManager::GetAsset<Texture2D>(self->GetRenderResource<RenderComponentTexture>("ScreenTexture")->TextureHandle)->Resize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
				AssetManager::GetAsset<Texture2D>(self->GetRenderResource<RenderComponentTexture>("AlternateScreenTexture")->TextureHandle)->Resize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
				self->GetRenderResource<RenderComponentPrimitiveType<BloomMipChain>>("BloomMipChain")->Data.Resize(viewportSize);
				self->GetRenderResource<RenderComponentFramebuffer>("BloomFramebuffer")->Framebuffer->Resize(viewportSize.x, viewportSize.y);
				return false;
			});
		};
		out_Framerenderer.SetEventFunc(eventFunc);

		// Render pass functions
		// ---------------------

		// { primitive<glm::ivec2>, Texture, primitive<Ref<Material>>, primitive<bool>, EnvironmentMap }
		RenderPass::OnRenderFunc scene3DPass = [](RenderPass::RenderPassContext& context, Ref<Framebuffer> targetFramebuffer, std::vector<IRenderComponent*> inputs) {
			PE_PROFILE_SCOPE("Scene 3D Render Pass");
			Ref<Camera> activeCamera = context.ActiveCamera;
			const glm::mat4& cameraWorldTransform = context.CameraWorldTransform;
			PE_CORE_ASSERT(inputs[0], "Viewport resolution input required");
			PE_CORE_ASSERT(inputs[1], "Target texture attachment input required");
			PE_CORE_ASSERT(inputs[2], "Preview Ref<Material> input required");
			PE_CORE_ASSERT(inputs[3], "Sphere flag input required");
			PE_CORE_ASSERT(inputs[4], "Environment map input required");
			RenderComponentPrimitiveType<glm::ivec2>* viewportResInput = dynamic_cast<RenderComponentPrimitiveType<glm::ivec2>*>(inputs[0]);
			RenderComponentTexture* screenTextureInput = dynamic_cast<RenderComponentTexture*>(inputs[1]);
			RenderComponentPrimitiveType<Ref<Material>>* previewMaterialInput = dynamic_cast<RenderComponentPrimitiveType<Ref<Material>>*>(inputs[2]);
			RenderComponentPrimitiveType<bool>* sphereFlagInput = dynamic_cast<RenderComponentPrimitiveType<bool>*>(inputs[3]);
			RenderComponentEnvironmentMap* envMapInput = dynamic_cast<RenderComponentEnvironmentMap*>(inputs[4]);

			// Ping - pong framebuffer attachment
			Ref<FramebufferAttachment> attach = targetFramebuffer->GetAttachment(FramebufferAttachmentPoint::Colour0);
			PE_CORE_ASSERT(attach->GetType() == FramebufferAttachmentType::Texture2D, "Invalid framebuffer attachment");
			AssetHandle screenTextureInputHandle = screenTextureInput->TextureHandle;
			AssetHandle currentTargetTexture = static_cast<FramebufferTexture2DAttachment*>(attach.get())->GetTextureHandle();
			if (currentTargetTexture != screenTextureInputHandle)
			{
				Ref<FramebufferTexture2DAttachment> attach = FramebufferTexture2DAttachment::Create(FramebufferAttachmentPoint::Colour0, screenTextureInputHandle);
				targetFramebuffer->AddColourAttachment(attach);
			}

			RenderCommand::SetViewport({ 0.0f, 0.0f }, viewportResInput->Data);
			RenderCommand::SetClearColour(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
			RenderCommand::Clear();

			Renderer::BeginScene(activeCamera->GetProjection(), cameraWorldTransform, activeCamera->GetGamma(), activeCamera->GetExposure());
			Renderer::PointLight pointLight;
			pointLight.Position = glm::vec4(0.0f, 0.0f, 0.0f, 25.0f);
			pointLight.Diffuse = glm::vec4(s_LightColour * s_LightIntensity, 1.0f);
			pointLight.Specular = glm::vec4(s_LightColour * s_LightIntensity, 1.0f);
			pointLight.Ambient = glm::vec4((s_LightColour * s_LightIntensity) * 0.1f, 1.0f);
			Renderer::SubmitPointLightSource(pointLight);

			if (envMapInput)
			{
				Ref<EnvironmentMap> envMap = AssetManager::GetAsset<EnvironmentMap>(envMapInput->EnvironmentHandle);
				AssetManager::GetAsset<TextureCubemap>(envMap->GetIrradianceMapHandle())->Bind(10);
				AssetManager::GetAsset<TextureCubemap>(envMap->GetPrefilteredMapHandle())->Bind(11);
				AssetManager::GetAsset<Texture2D>(EnvironmentMap::GetBRDFLutHandle())->Bind(12);
			}

			if (sphereFlagInput->Data)
			{
				Renderer::DrawDefaultSphereImmediate(previewMaterialInput->Data, s_MeshTransform, { DepthFunc::LEQUAL, true, true }, FaceCulling::BACK, BlendState());
			}
			else
			{
				Renderer::DrawDefaultCubeImmediate(previewMaterialInput->Data, s_MeshTransform, { DepthFunc::LEQUAL, true, true }, FaceCulling::BACK, BlendState());
			}
			Renderer::EndScene();
		};

		// { primitive<glm::ivec2>, Material, EnvironmentMap }
		RenderPass::OnRenderFunc skyboxPass = [](RenderPass::RenderPassContext& context, Ref<Framebuffer> targetFramebuffer, std::vector<IRenderComponent*> inputs)
		{
			PE_PROFILE_SCOPE("Skybox Render Pass");
			Ref<Scene>& sceneContext = context.ActiveScene;
			Ref<Camera> activeCamera = context.ActiveCamera;
			const glm::mat4& cameraWorldTransform = context.CameraWorldTransform;
			PE_CORE_ASSERT(inputs[0], "Viewport resolution input required");
			PE_CORE_ASSERT(inputs[1], "Skybox material input required");
			RenderComponentPrimitiveType<glm::ivec2>* viewportResInput = dynamic_cast<RenderComponentPrimitiveType<glm::ivec2>*>(inputs[0]);
			RenderComponentMaterial* skyboxMaterialInput = dynamic_cast<RenderComponentMaterial*>(inputs[1]);
			RenderComponentEnvironmentMap* envMapInput = dynamic_cast<RenderComponentEnvironmentMap*>(inputs[2]);

			Ref<Material> skyboxMaterial = AssetManager::GetAsset<Material>(skyboxMaterialInput->MaterialHandle);
			if (envMapInput)
			{
				// Apply environment map to skybox
				Ref<EnvironmentMap> envMap = AssetManager::GetAsset<EnvironmentMap>(envMapInput->EnvironmentHandle);
				skyboxMaterial->GetParameter<SamplerCubeShaderParameterTypeStorage>("Skybox")->TextureHandle = envMap->GetUnfilteredHandle();
			}

			// Remove translation from view matrix
			glm::mat4 cameraTransform = cameraWorldTransform;
			cameraTransform = glm::mat4(glm::mat3(cameraTransform));

			RenderCommand::SetViewport({ 0, 0 }, { viewportResInput->Data.x, viewportResInput->Data.y });
			targetFramebuffer->SetDrawBuffers({ FramebufferAttachmentPoint::Colour0 });
			Renderer::BeginScene(activeCamera->GetProjection(), cameraTransform, activeCamera->GetGamma(), activeCamera->GetExposure());

			DepthState depthState;
			depthState.Func = DepthFunc::LEQUAL;
			FaceCulling cullState = FaceCulling::FRONT;

			Renderer::DrawDefaultCubeImmediate(skyboxMaterial, glm::mat4(1.0f), depthState, cullState, BlendState(), -1);
			Renderer::EndScene();
		};

		// { primitive<glm::ivec2>, primitive<BloomMipChain>, Material, Texture }
		RenderPass::OnRenderFunc bloomDownsamplePass = [](RenderPass::RenderPassContext& context, Ref<Framebuffer> targetFramebuffer, std::vector<IRenderComponent*> inputs)
			{
				PE_PROFILE_SCOPE("Bloom Downsample Pass");
				PE_CORE_ASSERT(inputs[0], "Viewport resolution input required");
				PE_CORE_ASSERT(inputs[1], "Downsample mip chain input required");
				PE_CORE_ASSERT(inputs[2], "Downsample material input required");
				PE_CORE_ASSERT(inputs[3], "Source texture input required");
				Ref<Camera> activeCamera = context.ActiveCamera;
				const glm::mat4& cameraWorldTransform = context.CameraWorldTransform;
				RenderComponentPrimitiveType<glm::ivec2>* viewportResInput = dynamic_cast<RenderComponentPrimitiveType<glm::ivec2>*>(inputs[0]);
				RenderComponentPrimitiveType<BloomMipChain>* mipChainInput = dynamic_cast<RenderComponentPrimitiveType<BloomMipChain>*>(inputs[1]);
				RenderComponentMaterial* downsampleMaterialInput = dynamic_cast<RenderComponentMaterial*>(inputs[2]);
				RenderComponentTexture* sourceTextureInput = dynamic_cast<RenderComponentTexture*>(inputs[3]);

				glm::vec2 srcResolution = viewportResInput->Data;
				const float threshold = 1.0f;
				const float softThreshold = 0.5f;
				int FirstIteration = 1;
				const float gamma = activeCamera->GetGamma();

				Ref<Material> downsampleMaterial = AssetManager::GetAsset<Material>(downsampleMaterialInput->MaterialHandle);
				UniformBufferStorage* uboStorage = downsampleMaterial->GetParameter<UBOShaderParameterTypeStorage>("DownsampleData")->UBO().get();
				uboStorage->SetLocalData("SourceResolution", srcResolution);
				uboStorage->SetLocalData("Threshold", threshold);
				uboStorage->SetLocalData("SoftThreshold", softThreshold);
				uboStorage->SetLocalData<int>("FirstIteration", FirstIteration);
				uboStorage->SetLocalData("Gamma", gamma);

				// Progressively downsample through mip chain
				BloomMipChain& mipChain = mipChainInput->Data;
				Ref<Texture2D> source = AssetManager::GetAsset<Texture2D>(sourceTextureInput->TextureHandle);
				Texture* previousTexture = source.get();
				for (int i = 0; i < mipChain.Size(); i++)
				{
					Ref<Texture2D> mip = mipChain.GetMipLevel(i);
					if (i > 0)
					{
						uboStorage->SetLocalData<int>("FirstIteration", 0);
					}

					const TextureSpecification& mipSpec = mip->GetSpecification();
					RenderCommand::SetViewport({ 0, 0 }, { mipSpec.Width, mipSpec.Height });

					Ref<FramebufferTexture2DAttachment> attachment = FramebufferTexture2DAttachment::Create(FramebufferAttachmentPoint::Colour0, mip->Handle);
					targetFramebuffer->AddColourAttachment(attachment);
					RenderCommand::Clear();

					downsampleMaterial->GetParameter<Sampler2DShaderParameterTypeStorage>("SourceTexture")->TextureHandle = previousTexture->Handle;
					Renderer::BeginScene(activeCamera->GetProjection(), cameraWorldTransform, activeCamera->GetGamma(), activeCamera->GetExposure());
					BlendState blend;
					blend.Enabled = false;
					Renderer::DrawDefaultQuadImmediate(downsampleMaterial, glm::mat4(1.0f), { DepthFunc::ALWAYS, true, true }, FaceCulling::BACK, blend, -1);
					Renderer::EndScene();

					// Set resolution for next iteration to the currently written mip
					uboStorage->SetLocalData("SourceResolution", glm::vec2(mipSpec.Width, mipSpec.Height));
					previousTexture = mip.get();
				}
			};

		// { primitive<glm::ivec2>, primitive<BloomMipChain>, Material }
		RenderPass::OnRenderFunc bloomUpsamplePass = [](RenderPass::RenderPassContext& context, Ref<Framebuffer> targetFramebuffer, std::vector<IRenderComponent*> inputs) {
			PE_PROFILE_SCOPE("Bloom Upsample Pass");
			PE_CORE_ASSERT(inputs[0], "Viewport resolution input required");
			PE_CORE_ASSERT(inputs[1], "Upsample mip chain input required");
			PE_CORE_ASSERT(inputs[2], "Upsample material input required");
			Ref<Camera> activeCamera = context.ActiveCamera;
			const glm::mat4& cameraWorldTransform = context.CameraWorldTransform;
			RenderComponentPrimitiveType<glm::ivec2>* viewportResInput = dynamic_cast<RenderComponentPrimitiveType<glm::ivec2>*>(inputs[0]);
			RenderComponentPrimitiveType<BloomMipChain>* mipChainInput = dynamic_cast<RenderComponentPrimitiveType<BloomMipChain>*>(inputs[1]);
			RenderComponentMaterial* upsampleMaterialInput = dynamic_cast<RenderComponentMaterial*>(inputs[2]);

			glm::vec2 resolution = viewportResInput->Data;
			float FilterRadius = 0.005f;
			float AspectRatio = resolution.x / resolution.y;

			Ref<Material> upsampleMaterial = AssetManager::GetAsset<Material>(upsampleMaterialInput->MaterialHandle);
			UniformBufferStorage* uboStorage = upsampleMaterial->GetParameter<UBOShaderParameterTypeStorage>("UpsampleData")->UBO().get();
			uboStorage->SetLocalData("FilterRadius", FilterRadius);
			uboStorage->SetLocalData("AspectRatio", AspectRatio);

			BlendState blend;
			blend.Enabled = true;
			blend.SrcFactor = BlendFunc::ONE;
			blend.DstFactor = BlendFunc::ONE;
			blend.Equation = BlendEquation::ADD;

			// Progressively upsample through mip chain
			BloomMipChain& mipChain = mipChainInput->Data;
			for (int i = mipChain.Size() - 1; i > 0; i--)
			{
				Ref<Texture2D> mip = mipChain.GetMipLevel(i);
				Ref<Texture2D> nextMip = mipChain.GetMipLevel(i - 1);

				Ref<FramebufferTexture2DAttachment> attachment = FramebufferTexture2DAttachment::Create(FramebufferAttachmentPoint::Colour0, nextMip->Handle);
				targetFramebuffer->AddColourAttachment(attachment);

				const TextureSpecification& nexMipSpec = nextMip->GetSpecification();
				RenderCommand::SetViewport({ 0, 0 }, { nexMipSpec.Width, nexMipSpec.Height });

				upsampleMaterial->GetParameter<Sampler2DShaderParameterTypeStorage>("SourceTexture")->TextureHandle = mip->Handle;
				Renderer::BeginScene(activeCamera->GetProjection(), cameraWorldTransform, activeCamera->GetGamma(), activeCamera->GetExposure());
				Renderer::DrawDefaultQuadImmediate(upsampleMaterial, glm::mat4(1.0f), { DepthFunc::ALWAYS, true, true }, FaceCulling::BACK, blend, -1);
				Renderer::EndScene();
			}
			};

		// { primitive<glm::ivec2>, primitive<BloomMipChain>, Material, Texture, Texture, Texture }
		RenderPass::OnRenderFunc bloomCombinePass = [](RenderPass::RenderPassContext& context, Ref<Framebuffer> targetFramebuffer, std::vector<IRenderComponent*> inputs)
			{
				PE_PROFILE_SCOPE("Bloom Combine Pass");
				PE_CORE_ASSERT(inputs[0], "Viewport resolution input required");
				PE_CORE_ASSERT(inputs[1], "Bloom mip chain input required");
				PE_CORE_ASSERT(inputs[2], "Combine material input required");
				PE_CORE_ASSERT(inputs[3], "Dirt mask input required");
				PE_CORE_ASSERT(inputs[4], "Source texture input required");
				PE_CORE_ASSERT(inputs[5], "Target texture input required");
				Ref<Camera> activeCamera = context.ActiveCamera;
				const glm::mat4& cameraWorldTransform = context.CameraWorldTransform;
				RenderComponentPrimitiveType<glm::ivec2>* viewportResInput = dynamic_cast<RenderComponentPrimitiveType<glm::ivec2>*>(inputs[0]);
				RenderComponentPrimitiveType<BloomMipChain>* mipChainInput = dynamic_cast<RenderComponentPrimitiveType<BloomMipChain>*>(inputs[1]);
				RenderComponentMaterial* combineMaterialInput = dynamic_cast<RenderComponentMaterial*>(inputs[2]);
				RenderComponentTexture* dirtMaskTextureInput = dynamic_cast<RenderComponentTexture*>(inputs[3]);
				RenderComponentTexture* sourceTextureInput = dynamic_cast<RenderComponentTexture*>(inputs[4]);
				RenderComponentTexture* targetTextureInput = dynamic_cast<RenderComponentTexture*>(inputs[5]);

				// Ping - pong framebuffer attachment
				Ref<FramebufferAttachment> attach = targetFramebuffer->GetAttachment(FramebufferAttachmentPoint::Colour0);
				PE_CORE_ASSERT(attach->GetType() == FramebufferAttachmentType::Texture2D, "Invalid framebuffer attachment");
				AssetHandle targetTextureInputHandle = targetTextureInput->TextureHandle;
				AssetHandle currentTargetTexture = static_cast<FramebufferTexture2DAttachment*>(attach.get())->GetTextureHandle();
				if (currentTargetTexture != targetTextureInputHandle)
				{
					Ref<FramebufferTexture2DAttachment> attach = FramebufferTexture2DAttachment::Create(FramebufferAttachmentPoint::Colour0, targetTextureInputHandle);
					targetFramebuffer->AddColourAttachment(attach);
				}
				targetFramebuffer->SetDrawBuffers({ FramebufferAttachmentPoint::Colour0 });

				float BloomStrength = 0.04f;
				float DirtMaskStrength = 0.5f;
				int UseDirtMask = 1;

				Ref<Material> combineMaterial = AssetManager::GetAsset<Material>(combineMaterialInput->MaterialHandle);
				UniformBufferStorage* uboStorage = combineMaterial->GetParameter<UBOShaderParameterTypeStorage>("BloomCombineData")->UBO().get();
				uboStorage->SetLocalData("BloomStrength", BloomStrength);
				uboStorage->SetLocalData("DirtMaskStrength", DirtMaskStrength);
				uboStorage->SetLocalData("UseDirtMask", UseDirtMask);

				combineMaterial->GetParameter<Sampler2DShaderParameterTypeStorage>("ColourTexture")->TextureHandle = sourceTextureInput->TextureHandle;
				combineMaterial->GetParameter<Sampler2DShaderParameterTypeStorage>("BloomTexture")->TextureHandle = mipChainInput->Data.GetMipHandle(0);
				if (UseDirtMask)
				{
					combineMaterial->GetParameter<Sampler2DShaderParameterTypeStorage>("DirtMaskTexture")->TextureHandle = dirtMaskTextureInput->TextureHandle;
				}

				RenderCommand::SetViewport({ 0.0f, 0.0f }, viewportResInput->Data);
				Renderer::BeginScene(activeCamera->GetProjection(), cameraWorldTransform, activeCamera->GetGamma(), activeCamera->GetExposure());
				Renderer::DrawDefaultQuadImmediate(combineMaterial, glm::mat4(1.0f), { DepthFunc::ALWAYS, true, true }, FaceCulling::BACK, BlendState(), -1);
				Renderer::EndScene();
			};

		// { primitive<glm::ivec2>, Texture, Material, Texture }
		RenderPass::OnRenderFunc gammaTonemapPass = [](RenderPass::RenderPassContext& context, Ref<Framebuffer> targetFramebuffer, std::vector<IRenderComponent*> inputs) {
			PE_PROFILE_SCOPE("Scene 2D Render Pass");
			Ref<Camera> activeCamera = context.ActiveCamera;
			const glm::mat4& cameraWorldTransform = context.CameraWorldTransform;
			PE_CORE_ASSERT(inputs[0], "Viewport resolution input required");
			PE_CORE_ASSERT(inputs[1], "Target texture input required");
			PE_CORE_ASSERT(inputs[2], "Material input required");
			PE_CORE_ASSERT(inputs[3], "Source texture input required");
			RenderComponentPrimitiveType<glm::ivec2>* viewportResInput = dynamic_cast<RenderComponentPrimitiveType<glm::ivec2>*> (inputs[0]);
			RenderComponentTexture* screenTextureInput = dynamic_cast<RenderComponentTexture*>(inputs[1]);
			RenderComponentMaterial* gammaCorrectionMaterialInput = dynamic_cast<RenderComponentMaterial*>(inputs[2]);
			RenderComponentTexture* sourceTextureInput = dynamic_cast<RenderComponentTexture*>(inputs[3]);

			// Ping - pong framebuffer attachment
			Ref<FramebufferAttachment> attach = targetFramebuffer->GetAttachment(FramebufferAttachmentPoint::Colour0);
			PE_CORE_ASSERT(attach->GetType() == FramebufferAttachmentType::Texture2D, "Invalid framebuffer attachment");
			AssetHandle screenTextureInputHandle = screenTextureInput->TextureHandle;
			AssetHandle currentTargetTexture = static_cast<FramebufferTexture2DAttachment*>(attach.get())->GetTextureHandle();
			if (currentTargetTexture != screenTextureInputHandle)
			{
				Ref<FramebufferTexture2DAttachment> attach = FramebufferTexture2DAttachment::Create(FramebufferAttachmentPoint::Colour0, screenTextureInputHandle);
				targetFramebuffer->AddColourAttachment(attach);
			}

			targetFramebuffer->SetDrawBuffers({ FramebufferAttachmentPoint::Colour0 });
			RenderCommand::SetViewport({ 0.0f, 0.0f }, viewportResInput->Data);
			RenderCommand::SetClearColour(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
			RenderCommand::Clear();

			Ref<Material> material = AssetManager::GetAsset<Material>(gammaCorrectionMaterialInput->MaterialHandle);
			material->GetParameter<Sampler2DShaderParameterTypeStorage>("SourceTexture")->TextureHandle = sourceTextureInput->TextureHandle;

			Renderer::BeginScene(activeCamera->GetProjection(), cameraWorldTransform, activeCamera->GetGamma(), activeCamera->GetExposure());
			BlendState blend;
			blend.Enabled = false;
			Renderer::DrawDefaultQuadImmediate(material, glm::mat4(1.0f), { DepthFunc::ALWAYS, true, true }, FaceCulling::BACK, blend, -1);
			Renderer::EndScene();
		};

		// Add render passes
		// -----------------

		// Main render
		out_Framerenderer.AddRenderPass(RenderPass({ RenderComponentType::PrimitiveType, RenderComponentType::Texture, RenderComponentType::PrimitiveType, RenderComponentType::PrimitiveType, RenderComponentType::EnvironmentMap }, scene3DPass), m_Framebuffer, { "ViewportResolution", "ScreenTexture", "PreviewMaterial", "SphereSelected", "EnvironmentMap" });
		out_Framerenderer.AddRenderPass(RenderPass({ RenderComponentType::PrimitiveType, RenderComponentType::Material, RenderComponentType::EnvironmentMap }, skyboxPass), m_Framebuffer, { "ViewportResolution", "SkyboxMaterial", "EnvironmentMap" });

		// Bloom
		out_Framerenderer.AddRenderPass(RenderPass({ RenderComponentType::PrimitiveType, RenderComponentType::PrimitiveType, RenderComponentType::Material, RenderComponentType::Texture }, bloomDownsamplePass), bloomFBO, { "ViewportResolution", "BloomMipChain", "MipChainDownsampleMaterial", "ScreenTexture" });
		out_Framerenderer.AddRenderPass(RenderPass({ RenderComponentType::PrimitiveType, RenderComponentType::PrimitiveType, RenderComponentType::Material }, bloomUpsamplePass), bloomFBO, { "ViewportResolution", "BloomMipChain", "MipChainUpsampleMaterial" });
		out_Framerenderer.AddRenderPass(RenderPass({ RenderComponentType::PrimitiveType, RenderComponentType::PrimitiveType, RenderComponentType::Material, RenderComponentType::Texture, RenderComponentType::Texture, RenderComponentType::Texture }, bloomCombinePass), m_Framebuffer, { "ViewportResolution", "BloomMipChain", "BloomCombineMaterial", "DirtMaskTexture", "ScreenTexture", "AlternateScreenTexture" });

		// Post process
		out_Framerenderer.AddRenderPass(RenderPass({ RenderComponentType::PrimitiveType, RenderComponentType::Texture, RenderComponentType::Material, RenderComponentType::Texture }, gammaTonemapPass), m_Framebuffer, { "ViewportResolution", "ScreenTexture", "GammaTonemapMaterial", "AlternateScreenTexture" });
	}

	void CreateMaterialWindow::CreateRawRenderer(FrameRenderer& out_Framerenderer)
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
		Ref<Texture2D> screenTexture = AssetManager::CreateAsset<Texture2D>(true, screenSpec);
		Ref<Texture2D> alternateScreenTexture = AssetManager::CreateAsset<Texture2D>(true, screenSpec);
		out_Framerenderer.AddRenderResource<RenderComponentTexture>("ScreenTexture", false, screenTexture->Handle);
		out_Framerenderer.AddRenderResource<RenderComponentTexture>("AlternateScreenTexture", false, alternateScreenTexture->Handle);

		Ref<FramebufferTexture2DAttachment> screenAttachment = FramebufferTexture2DAttachment::Create(FramebufferAttachmentPoint::Colour0, screenTexture->Handle);

		m_Framebuffer->AddColourAttachment(screenAttachment);

		//  Data
		// ------
		glm::ivec2 viewportRes = { (glm::ivec2)m_ViewportSize };
		out_Framerenderer.AddRenderResource<RenderComponentPrimitiveType<glm::ivec2>>("ViewportResolution", false, viewportRes);

		Ref<Material> material = m_Material;
		out_Framerenderer.AddRenderResource<RenderComponentPrimitiveType<Ref<Material>>>("PreviewMaterial", false, material);

		bool sphereSelected = m_SphereSelected;
		out_Framerenderer.AddRenderResource<RenderComponentPrimitiveType<bool>>("SphereSelected", false, sphereSelected);

		out_Framerenderer.AddRenderResource<RenderComponentFramebuffer>("MainFramebuffer", false, m_Framebuffer);

		// Materials
		// ---------
		Ref<EditorAssetManager> assetManager = Project::GetActive()->GetEditorAssetManager();
		std::filesystem::path engineAssetsRelativeToProjectAssets = std::filesystem::path("assets").lexically_relative(Project::GetAssetDirectory());

		AssetHandle gammaTonemapShaderHandle = assetManager->ImportAssetFromFile(engineAssetsRelativeToProjectAssets / "shaders/GammaTonemap.glsl", true);
		Ref<Material> gammaTonemapMaterial = AssetManager::CreateAsset<Material>(true, gammaTonemapShaderHandle);

		out_Framerenderer.AddRenderResource<RenderComponentMaterial>("GammaTonemapMaterial", false, gammaTonemapMaterial->Handle);

		// OnEvent
		// -------
		FrameRenderer::OnEventFunc eventFunc = [](Event& e, FrameRenderer* self)
			{
				EventDispatcher dispatcher = EventDispatcher(e);
				dispatcher.DispatchEvent<MainViewportResizeEvent>([self](MainViewportResizeEvent& e)->bool {
					glm::ivec2 viewportSize = glm::ivec2(e.GetWidth(), e.GetHeight());
					self->GetRenderResource<RenderComponentPrimitiveType<glm::ivec2>>("ViewportResolution")->Data = viewportSize;
					AssetManager::GetAsset<Texture2D>(self->GetRenderResource<RenderComponentTexture>("ScreenTexture")->TextureHandle)->Resize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
					AssetManager::GetAsset<Texture2D>(self->GetRenderResource<RenderComponentTexture>("AlternateScreenTexture")->TextureHandle)->Resize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
					return false;
				});
			};
		out_Framerenderer.SetEventFunc(eventFunc);

		// Render pass functions
		// ---------------------

		// { primitive<glm::ivec2>, Texture, primitive<Ref<Material>>, primitive<bool> }
		RenderPass::OnRenderFunc scene3DPass = [](RenderPass::RenderPassContext& context, Ref<Framebuffer> targetFramebuffer, std::vector<IRenderComponent*> inputs) {
			PE_PROFILE_SCOPE("Scene 3D Render Pass");
			Ref<Camera> activeCamera = context.ActiveCamera;
			const glm::mat4& cameraWorldTransform = context.CameraWorldTransform;
			PE_CORE_ASSERT(inputs[0], "Viewport resolution input required");
			PE_CORE_ASSERT(inputs[1], "Target texture attachment input required");
			PE_CORE_ASSERT(inputs[2], "Preview Ref<Material> input required");
			PE_CORE_ASSERT(inputs[3], "Sphere flag input required");
			RenderComponentPrimitiveType<glm::ivec2>* viewportResInput = dynamic_cast<RenderComponentPrimitiveType<glm::ivec2>*>(inputs[0]);
			RenderComponentTexture* screenTextureInput = dynamic_cast<RenderComponentTexture*>(inputs[1]);
			RenderComponentPrimitiveType<Ref<Material>>* previewMaterialInput = dynamic_cast<RenderComponentPrimitiveType<Ref<Material>>*>(inputs[2]);
			RenderComponentPrimitiveType<bool>* sphereFlagInput = dynamic_cast<RenderComponentPrimitiveType<bool>*>(inputs[3]);

			// Ping - pong framebuffer attachment
			Ref<FramebufferAttachment> attach = targetFramebuffer->GetAttachment(FramebufferAttachmentPoint::Colour0);
			PE_CORE_ASSERT(attach->GetType() == FramebufferAttachmentType::Texture2D, "Invalid framebuffer attachment");
			AssetHandle screenTextureInputHandle = screenTextureInput->TextureHandle;
			AssetHandle currentTargetTexture = static_cast<FramebufferTexture2DAttachment*>(attach.get())->GetTextureHandle();
			if (currentTargetTexture != screenTextureInputHandle)
			{
				Ref<FramebufferTexture2DAttachment> attach = FramebufferTexture2DAttachment::Create(FramebufferAttachmentPoint::Colour0, screenTextureInputHandle);
				targetFramebuffer->AddColourAttachment(attach);
			}

			RenderCommand::SetViewport({ 0.0f, 0.0f }, viewportResInput->Data);
			RenderCommand::SetClearColour(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
			RenderCommand::Clear();

			Renderer::BeginScene(activeCamera->GetProjection(), cameraWorldTransform, activeCamera->GetGamma(), activeCamera->GetExposure());
			Renderer::PointLight pointLight;
			pointLight.Position = glm::vec4(0.0f, 0.0f, 0.0f, 25.0f);
			pointLight.Diffuse = glm::vec4(s_LightColour * s_LightIntensity, 1.0f);
			pointLight.Specular = glm::vec4(s_LightColour * s_LightIntensity, 1.0f);
			pointLight.Ambient = glm::vec4((s_LightColour * s_LightIntensity) * 0.1f, 1.0f);
			Renderer::SubmitPointLightSource(pointLight);

			if (sphereFlagInput->Data)
			{
				Renderer::DrawDefaultSphereImmediate(previewMaterialInput->Data, s_MeshTransform, { DepthFunc::LEQUAL, true, true }, FaceCulling::BACK, BlendState());
			}
			else
			{
				Renderer::DrawDefaultCubeImmediate(previewMaterialInput->Data, s_MeshTransform, { DepthFunc::LEQUAL, true, true }, FaceCulling::BACK, BlendState());
			}
			Renderer::EndScene();
			};

		// { primitive<glm::ivec2>, Texture, Material, Texture }
		RenderPass::OnRenderFunc gammaTonemapPass = [](RenderPass::RenderPassContext& context, Ref<Framebuffer> targetFramebuffer, std::vector<IRenderComponent*> inputs) {
			PE_PROFILE_SCOPE("Scene 2D Render Pass");
			Ref<Camera> activeCamera = context.ActiveCamera;
			const glm::mat4& cameraWorldTransform = context.CameraWorldTransform;
			PE_CORE_ASSERT(inputs[0], "Viewport resolution input required");
			PE_CORE_ASSERT(inputs[1], "Target texture input required");
			PE_CORE_ASSERT(inputs[2], "Material input required");
			PE_CORE_ASSERT(inputs[3], "Source texture input required");
			RenderComponentPrimitiveType<glm::ivec2>* viewportResInput = dynamic_cast<RenderComponentPrimitiveType<glm::ivec2>*> (inputs[0]);
			RenderComponentTexture* screenTextureInput = dynamic_cast<RenderComponentTexture*>(inputs[1]);
			RenderComponentMaterial* gammaCorrectionMaterialInput = dynamic_cast<RenderComponentMaterial*>(inputs[2]);
			RenderComponentTexture* sourceTextureInput = dynamic_cast<RenderComponentTexture*>(inputs[3]);

			// Ping - pong framebuffer attachment
			Ref<FramebufferAttachment> attach = targetFramebuffer->GetAttachment(FramebufferAttachmentPoint::Colour0);
			PE_CORE_ASSERT(attach->GetType() == FramebufferAttachmentType::Texture2D, "Invalid framebuffer attachment");
			AssetHandle screenTextureInputHandle = screenTextureInput->TextureHandle;
			AssetHandle currentTargetTexture = static_cast<FramebufferTexture2DAttachment*>(attach.get())->GetTextureHandle();
			if (currentTargetTexture != screenTextureInputHandle)
			{
				Ref<FramebufferTexture2DAttachment> attach = FramebufferTexture2DAttachment::Create(FramebufferAttachmentPoint::Colour0, screenTextureInputHandle);
				targetFramebuffer->AddColourAttachment(attach);
			}

			targetFramebuffer->SetDrawBuffers({ FramebufferAttachmentPoint::Colour0 });
			RenderCommand::SetViewport({ 0.0f, 0.0f }, viewportResInput->Data);
			RenderCommand::SetClearColour(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
			RenderCommand::Clear();

			Ref<Material> material = AssetManager::GetAsset<Material>(gammaCorrectionMaterialInput->MaterialHandle);
			material->GetParameter<Sampler2DShaderParameterTypeStorage>("SourceTexture")->TextureHandle = sourceTextureInput->TextureHandle;

			Renderer::BeginScene(activeCamera->GetProjection(), cameraWorldTransform, activeCamera->GetGamma(), activeCamera->GetExposure());
			BlendState blend;
			blend.Enabled = false;
			Renderer::DrawDefaultQuadImmediate(material, glm::mat4(1.0f), { DepthFunc::ALWAYS, true, true }, FaceCulling::BACK, blend, -1);
			//Renderer::SubmitDefaultQuad(gammaCorrectionMaterialInput->MaterialHandle, glm::mat4(1.0f), { DepthFunc::ALWAYS, true, true }, FaceCulling::BACK, blend, -1);
			Renderer::EndScene();
		};


		// Add render passes
		// -----------------

		// Main render
		out_Framerenderer.AddRenderPass(RenderPass({ RenderComponentType::PrimitiveType, RenderComponentType::Texture, RenderComponentType::PrimitiveType, RenderComponentType::PrimitiveType }, scene3DPass), m_Framebuffer, { "ViewportResolution", "AlternateScreenTexture", "PreviewMaterial", "SphereSelected" });
		// Post process
		out_Framerenderer.AddRenderPass(RenderPass({ RenderComponentType::PrimitiveType, RenderComponentType::Texture, RenderComponentType::Material, RenderComponentType::Texture }, gammaTonemapPass), m_Framebuffer, { "ViewportResolution", "ScreenTexture", "GammaTonemapMaterial", "AlternateScreenTexture" });
	}

	void CreateMaterialWindow::Init()
	{
		m_IconSphere = TextureImporter::LoadTexture2D("Resources/Icons/mingcute--shadow-fill.png");
		m_IconCube = TextureImporter::LoadTexture2D("Resources/Icons/mingcute--box-3-fill.png");

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
		Ref<FramebufferTexture2DAttachment> entityIDAttach = FramebufferTexture2DAttachment::Create(FramebufferAttachmentPoint::Colour1, texSpec, true);

		texSpec.Format = ImageFormat::Depth24Stencil8;
		Ref<FramebufferTexture2DAttachment> depthAttach = FramebufferTexture2DAttachment::Create(FramebufferAttachmentPoint::DepthStencil, texSpec, true);

		m_Framebuffer = Framebuffer::Create(spec, { entityIDAttach }, depthAttach);

		m_Framebuffer->AddColourAttachment(entityIDAttach);
		m_Framebuffer->SetDepthAttachment(depthAttach);

		m_Camera = CreateRef<SceneCamera>(SCENE_CAMERA_PERSPECTIVE);

		s_MeshTransform = glm::mat4(1.0f);
		s_MeshTransform = glm::translate(s_MeshTransform, glm::vec3(0.0f, 0.0f, -1.5f));
		s_MeshTransform = glm::rotate(s_MeshTransform, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		m_Renderer = FrameRenderer();
		CreateRenderer(m_Renderer);
	}

	void CreateMaterialWindow::SetContext(AssetHandle shaderHandle)
	{
		m_ShaderHandle = shaderHandle;
	}

	void CreateMaterialWindow::OnImGuiRender()
	{
		PE_PROFILE_FUNCTION();
		if (m_ShowWindow) {
			ImGui::Begin("Create material asset");

			std::string label = "None";
			bool isShaderValid = false;
			if (m_ShaderHandle != 0)
			{
				if (AssetManager::IsAssetHandleValid(m_ShaderHandle) && AssetManager::GetAssetType(m_ShaderHandle) == AssetType::Shader)
				{
					const AssetMetadata& metadata = Project::GetActive()->GetEditorAssetManager()->GetMetadata(m_ShaderHandle);
					label = metadata.FilePath.filename().string();
					isShaderValid = true;
				}
				else { label = "Invalid"; }
			}

			// Engine shader drop down
			Ref<EditorAssetManager> assetManager = Project::GetActive()->GetEditorAssetManager();
			std::filesystem::path engineAssetsRelativeToProjectAssets = std::filesystem::path("assets").lexically_relative(Project::GetAssetDirectory());

			static std::unordered_map<const char*, std::filesystem::path> nameToFilepath;
			nameToFilepath["DefaultLit"] = engineAssetsRelativeToProjectAssets / "shaders/Renderer3D_DefaultLit.glsl";
			nameToFilepath["DefaultLitPBR"] = engineAssetsRelativeToProjectAssets / "shaders/Renderer3D_DefaultLitPBR.glsl";

			static const char* shaderNames[] = {
				"DefaultLit",
				"DefaultLitPBR"
			};

			if (m_DropDownShader != -1) {
				label = shaderNames[m_DropDownShader];
			}
			if (ImGui::BeginCombo("##EngineShaderDropDown", label.c_str())) {

				for (int i = 0; i < 2; i++) {
					bool isSelected = m_DropDownShader == i;
					if (ImGui::Selectable(shaderNames[i], isSelected)) {
						m_DropDownShader = i;
						m_ShaderHandle = assetManager->ImportAssetFromFile(nameToFilepath[shaderNames[i]], true);
						m_Material = CreateRef<Material>(m_ShaderHandle, true);
						auto renderResource = m_Renderer.GetRenderResource<RenderComponentPrimitiveType<Ref<Material>>>("PreviewMaterial");
						if (renderResource) { renderResource->Data = m_Material; }
					}

					if (isSelected) {
						ImGui::SetItemDefaultFocus();
					}
				}

				ImGui::EndCombo();
			}
			
			// Custom shader drag/drop target
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					AssetHandle handle = *(AssetHandle*)payload->Data;
					if (AssetManager::GetAssetType(handle) == AssetType::Shader) {
						m_ShaderHandle = handle;
						m_DropDownShader = -1;
						m_Material = CreateRef<Material>(m_ShaderHandle, true);
						auto renderResource = m_Renderer.GetRenderResource<RenderComponentPrimitiveType<Ref<Material>>>("PreviewMaterial");
						if (renderResource) { renderResource->Data = m_Material; }
					}
					else {
						PE_CORE_WARN("Invalid asset type. Shader needed for material creation");
					}
				}
				ImGui::EndDragDropTarget();
			}
			ImGui::SameLine();
			ImGui::Text("Material shader");
			
			ImGui::SameLine();
			DrawMeshIcons();

			// Left
			{
				ImGui::BeginChild("left pane", ImVec2(150, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
				// Material edit
				if (isShaderValid) {
					const Ref<Shader> shaderAsset = AssetManager::GetAsset<Shader>(m_ShaderHandle);
					int index = 0;
					for (auto& it : m_Material->m_ParameterMap) {
						Ref<ShaderParamaterTypeStorageBase> parameter = (it.second.IsBinding) ? m_Material->m_BindingParameters[it.second.Index] : m_Material->m_IndirectParameters[it.second.Index];

						ImGui::PushID(index);
						switch (parameter->GetType()) {
							case ShaderParameterType::UBO:
							{
								UBOShaderParameterTypeStorage* ubo = static_cast<UBOShaderParameterTypeStorage*>(parameter.get());
								LocalShaderBuffer& localBuffer = ubo->UBO()->GetLocalBuffer();
								DrawLocalShaderBufferMembersEdit(it.first, localBuffer);
								break;
							}
							case ShaderParameterType::SSBO:
							{
								StorageBufferEntryShaderParameterTypeStorage* ssbo = static_cast<StorageBufferEntryShaderParameterTypeStorage*>(parameter.get());
								LocalShaderBuffer& localBuffer = ssbo->GetLocalBuffer();
								DrawLocalShaderBufferMembersEdit(it.first, localBuffer);
								break;
							}
							case ShaderParameterType::Sampler2D:
							{
								Sampler2DShaderParameterTypeStorage* sampler2D = static_cast<Sampler2DShaderParameterTypeStorage*>(parameter.get());
								DrawSampler2DEdit(it.first, *sampler2D);
								break;
							}
							case ShaderParameterType::Sampler2DArray:
							{
								Sampler2DArrayShaderParameterTypeStorage* sampler2Darray = static_cast<Sampler2DArrayShaderParameterTypeStorage*>(parameter.get());
								DrawSampler2DArrayEdit(it.first, *sampler2Darray);
								break;
							}
						}
						ImGui::PopID();
						index++;
					}
				}
				ImGui::EndChild();
			}
			ImGui::SameLine();
			// Right
			{
				ImGui::BeginGroup();
				ImGui::BeginChild("preview", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
				ImGui::SeparatorText("Material Preview");
				ImGui::ColorEdit3("Light Colour", &s_LightColour[0], ImGuiColorEditFlags_DisplayRGB);
				ImGui::DragFloat("Light Intensity", &s_LightIntensity);

				if (isShaderValid) {
					ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
					if (m_ViewportSize.x != viewportPanelSize.x || m_ViewportSize.y != viewportPanelSize.y)
					{
						m_ViewportSize = glm::vec2(viewportPanelSize.x, viewportPanelSize.y);
						m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
						m_Camera->SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);

						MainViewportResizeEvent e = MainViewportResizeEvent((uint32_t)viewportPanelSize.x, (uint32_t)viewportPanelSize.y);
						m_Renderer.OnEvent(e);
					}

					Renderer::ResetStats();
					m_Framebuffer->Bind();
					m_Renderer.RenderFrame(nullptr, m_Camera, glm::mat4(1.0f));
					m_Framebuffer->Unbind();

					FramebufferTexture2DAttachment* texAttachment = dynamic_cast<FramebufferTexture2DAttachment*>(m_Framebuffer->GetAttachment(FramebufferAttachmentPoint::Colour0).get());
					Ref<Texture2D> textureAsset = texAttachment->GetTexture();
					uint32_t textureID = 0;
					if (textureAsset)
					{
						textureID = textureAsset->GetRendererID();
					}
					ImGui::Image(textureID, ImVec2(m_ViewportSize.x, m_ViewportSize.y), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
				}

				ImGui::EndChild();

				// Save or cancel
				if (ImGui::Button("Cancel")) {
					m_ShowWindow = false;
					m_Material = nullptr;
					auto renderResource = m_Renderer.GetRenderResource<RenderComponentPrimitiveType<Ref<Material>>>("PreviewMaterial");
					if (renderResource) { renderResource->Data = m_Material; }
					m_ShaderHandle = 0;
					m_DropDownShader = -1;
				}
				ImGui::SameLine();
				if (isShaderValid && ImGui::Button("Save As...")) {
					std::string path = FileDialogs::SaveFile("Paul Engine Material (*.pmat)\0*.pmat\0");
					if (!path.empty()) {
						std::filesystem::path absoluteProjectPath = std::filesystem::absolute(Project::GetProjectDirectory());
						std::filesystem::path relativeSavePath = std::filesystem::path(path).lexically_relative(absoluteProjectPath.parent_path());

						MaterialImporter::SaveMaterial(m_Material, relativeSavePath);
						Project::GetActive()->GetEditorAssetManager()->ImportAssetFromFile(relativeSavePath.lexically_relative(Project::GetAssetDirectory()), false);
					
						m_ShowWindow = false;
						m_Material = nullptr;
						auto renderResource = m_Renderer.GetRenderResource<RenderComponentPrimitiveType<Ref<Material>>>("PreviewMaterial");
						if (renderResource) { renderResource->Data = m_Material; }
						m_ShaderHandle = 0;
						m_DropDownShader = -1;
					}
				}

				ImGui::EndGroup();
			}

			ImGui::End();
		}
	}

	void CreateMaterialWindow::DrawMeshIcons()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		auto& colours = ImGui::GetStyle().Colors;
		auto& hovered = colours[ImGuiCol_ButtonHovered];
		auto& active = colours[ImGuiCol_ButtonActive];
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(hovered.x, hovered.y, hovered.z, 0.5f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(active.x, active.y, active.z, 0.5f));

		float size = ImGui::GetFrameHeight() * 1.2f;
		if (ImGui::ImageButton("##sphere_icon", m_IconSphere->GetRendererID(), ImVec2(size, size), ImVec2(0, 1), ImVec2(1, 0), ImVec4(0.0f, 0.0f, 0.0f, 0.0f), m_SphereSelected ? s_IconTintColourSelected : s_IconTintColour)) {
			m_SphereSelected = true;
			auto renderResource = m_Renderer.GetRenderResource<RenderComponentPrimitiveType<bool>>("SphereSelected");
			if (renderResource) { renderResource->Data = m_SphereSelected; }
		}
		ImGui::SetItemTooltip("Sphere preview");

		ImGui::SameLine();

		if (ImGui::ImageButton("##cube_icon", m_IconCube->GetRendererID(), ImVec2(size, size), ImVec2(0, 1), ImVec2(1, 0), ImVec4(0.0f, 0.0f, 0.0f, 0.0f), !m_SphereSelected ? s_IconTintColourSelected : s_IconTintColour)) {
			m_SphereSelected = false;
			auto renderResource = m_Renderer.GetRenderResource<RenderComponentPrimitiveType<bool>>("SphereSelected");
			if (renderResource) { renderResource->Data = m_SphereSelected; }
		}
		ImGui::SetItemTooltip("Cube preview");

		ImGui::PopStyleVar(1);
		ImGui::PopStyleColor(3);
	}

	void CreateMaterialWindow::DrawSampler2DEdit(const std::string& param_name, Sampler2DShaderParameterTypeStorage& sampler)
	{
		ImGui::Text(param_name.c_str());
		AssetHandle textureHandle = sampler.TextureHandle;

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
					sampler.TextureHandle = handle;
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
				sampler.TextureHandle = 0;
			}
		}
		ImGui::SameLine();
		ImGui::Text("Texture");
		ImGui::SameLine();
		ImGui::BeginDisabled(true);
		ImGui::Text("    binding: {0}", sampler.m_Binding);
		ImGui::EndDisabled();
	}

	void CreateMaterialWindow::DrawSampler2DArrayEdit(const std::string& param_name, Sampler2DArrayShaderParameterTypeStorage& samplerArray)
	{
		ImGui::Text(param_name.c_str());

		AssetHandle textureHandle = samplerArray.TextureArrayHandle;

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
					samplerArray.TextureArrayHandle = handle;
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
				samplerArray.TextureArrayHandle = 0;
			}
		}
		ImGui::SameLine();
		ImGui::Text("Texture Array");
		ImGui::SameLine();
		ImGui::BeginDisabled(true);
		ImGui::Text("    binding: {0}", samplerArray.m_Binding);
		ImGui::EndDisabled();
	}

	void CreateMaterialWindow::DrawLocalShaderBufferMembersEdit(const std::string& param_name, LocalShaderBuffer& localBuffer)
	{
		auto& layout = localBuffer.GetMembers();

		if (layout.size() > 0)
		{
			ImGui::Text(param_name.c_str());
		}
		for (const BufferElement& e : layout) {
			const std::string& name = e.Name;
			
			const std::string& commonPrefix = localBuffer.GetCommonNamePrefix();
			
			std::string displayName = name;
			if (commonPrefix != "")
			{
				displayName = name.substr(commonPrefix.size(), name.size() - commonPrefix.size());
			}

			const ShaderDataType type = e.Type;
			switch (type) {
				case ShaderDataType::Float4:
				{
					glm::vec4 data = glm::vec4(0.0f);
					localBuffer.ReadLocalMemberAs(name, data);
					if (ImGui::ColorEdit4(displayName.c_str(), &data[0]))
					{
						localBuffer.SetLocalMember(name, data);
					}
					break;
				}
				case ShaderDataType::Float3:
				{
					glm::vec3 data = glm::vec3(0.0f);
					localBuffer.ReadLocalMemberAs(name, data);
					if (ImGui::ColorEdit3(displayName.c_str(), &data[0]))
					{
						localBuffer.SetLocalMember(name, data);
					}
					break;
				}
				case ShaderDataType::Float2:
				{
					glm::vec2 data = glm::vec2(0.0f);
					localBuffer.ReadLocalMemberAs(name, data);
					if (ImGui::DragFloat2(displayName.c_str(), &data[0], 0.1f))
					{
						localBuffer.SetLocalMember(name, data);
					}
					break;
				}
				case ShaderDataType::Float:
				{
					float data = 0.0f;
					localBuffer.ReadLocalMemberAs(name, data);
					if (ImGui::DragFloat(displayName.c_str(), &data, 0.01f))
					{
						localBuffer.SetLocalMember(name, data);
					}
					break;
				}
				case ShaderDataType::Int4:
				{
					glm::ivec4 data = glm::ivec4(0);
					localBuffer.ReadLocalMemberAs(name, data);
					if (ImGui::DragInt4(displayName.c_str(), &data[0]))
					{
						localBuffer.SetLocalMember(name, data);
					}
					break;
				}
				case ShaderDataType::Int3:
				{
					glm::ivec3 data = glm::ivec3(0);
					localBuffer.ReadLocalMemberAs(name, data);
					if (ImGui::DragInt3(displayName.c_str(), &data[0]))
					{
						localBuffer.SetLocalMember(name, data);
					}
					break;
				}
				case ShaderDataType::Int2:
				{
					glm::ivec2 data = glm::ivec2(0);
					localBuffer.ReadLocalMemberAs(name, data);
					if (ImGui::DragInt2(displayName.c_str(), &data[0]))
					{
						localBuffer.SetLocalMember(name, data);
					}
					break;
				}
				case ShaderDataType::Int:
				{
					int data = 0;
					localBuffer.ReadLocalMemberAs(name, data);
					if (ImGui::DragInt(displayName.c_str(), &data))
					{
						localBuffer.SetLocalMember(name, data);
					}
					break;
				}
				case ShaderDataType::Bool:
				{
					bool data = false;
					localBuffer.ReadLocalMemberAs(name, data);
					if (ImGui::Checkbox(displayName.c_str(), &data))
					{
						localBuffer.SetLocalMember(name, data);
					}
					break;
				}
				case ShaderDataType::Sampler2DHandle:
				{
					DrawTextureHandleMemberEdit(name, displayName, localBuffer, AssetType::Texture2D);
					break;
				}
				case ShaderDataType::Sampler2DArrayHandle:
				{
					DrawTextureHandleMemberEdit(name, displayName, localBuffer, AssetType::Texture2DArray);
					break;
				}
				case ShaderDataType::SamplerCubeHandle:
				{
					DrawTextureHandleMemberEdit(name, displayName, localBuffer, AssetType::TextureCubemap);
					break;
				}
				case ShaderDataType::SamplerCubeArrayHandle:
				{
					DrawTextureHandleMemberEdit(name, displayName, localBuffer, AssetType::TextureCubemapArray);
					break;
				}
			}
		}
	}

	void CreateMaterialWindow::DrawTextureHandleMemberEdit(const std::string& member_name, const std::string& display_name, LocalShaderBuffer& localBuffer, AssetType textureType)
	{
		PE_CORE_ASSERT(Asset::IsTextureType(textureType), "Invalid asset type");

		ImGui::Text(display_name.c_str());

		// Read member
		AssetHandle textureHandle = 0;
		localBuffer.ReadLocalMemberAs(member_name, textureHandle);

		std::string label = "None";
		bool isTextureValid = false;
		if (textureHandle != 0) {
			if (AssetManager::IsAssetHandleValid(textureHandle) && AssetManager::GetAssetType(textureHandle) == textureType) {
				const AssetMetadata& metadata = AssetManager::GetMetadata(textureHandle);
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

		ImGui::PushID(member_name.c_str());
		ImGui::Button(label.c_str(), ImVec2(buttonLabelWidth, 0.0f));
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				AssetHandle handle = *(AssetHandle*)payload->Data;
				if (AssetManager::GetAssetType(handle) == textureType) {
					localBuffer.SetLocalMember(member_name, handle);
					// Load texture to make sure the device handle is generated
					AssetManager::GetAsset<Texture>(handle);
				}
				else {
					PE_CORE_WARN("Invalid asset type. {0} needed", AssetTypeToString(textureType));
				}
			}
			ImGui::EndDragDropTarget();
		}

		if (isTextureValid) {
			ImGui::SameLine();
			ImVec2 xLabelSize = ImGui::CalcTextSize("X");
			float buttonSize = xLabelSize.y + ImGui::GetStyle().FramePadding.y * 2.0f;
			if (ImGui::Button("X", ImVec2(buttonSize, buttonSize))) {
				AssetHandle newHandle = 0;
				localBuffer.SetLocalMember(member_name, newHandle);
			}
		}
		ImGui::SameLine();
		ImGui::Text("Texture");
		ImGui::PopID();
	}
}