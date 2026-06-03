#include "pepch.h"
#include "RenderBuilder.h"
#include "PaulEngine/Renderer/Asset/EnvironmentMap.h"
#include "PaulEngine/Renderer/RenderCommand.h"

namespace PaulEngine
{
	void RenderBuilder::BuildBasicForwardRenderer(Ref<FrameRenderer> out_Framerenderer)
	{
	}

	RenderBuilder::PrebuiltRenderFunc RenderBuilder::BuildDirLightShadowPass(Ref<FrameRenderer> out_Framerenderer)
	{
		static RenderPass::OnRenderFunc dirLightShadowPassFunc = 
			[](RenderPass::RenderPassContext& context, Ref<Framebuffer> targetFramebuffer, std::vector<IRenderComponent*> inputs) {
			PE_PROFILE_SCOPE("Directional light shadow map capture pass");
			Ref<Scene>& sceneContext = context.ActiveScene;
			RenderComponentPrimitiveType<glm::uvec2>* shadowResInput = (inputs[0]) ? dynamic_cast<RenderComponentPrimitiveType<glm::uvec2>*>(inputs[0]) : nullptr;
			RenderComponentMaterial* shadowmapMaterialInput = (inputs[1]) ? dynamic_cast<RenderComponentMaterial*>(inputs[1]) : nullptr;

			glm::uvec2 shadowResolution = (shadowResInput) ? shadowResInput->Data : glm::uvec2(1024u, 1024u);
			Ref<Material> shadowmapMaterial = (shadowmapMaterialInput) ? AssetManager::GetAsset<Material>(shadowmapMaterialInput->MaterialHandle) : nullptr;

			PE_CORE_ASSERT(shadowmapMaterial, "Invalid directional light shadow map material");

			Ref<FramebufferAttachment> depthAttachment = targetFramebuffer->GetDepthAttachment();
			PE_CORE_ASSERT(depthAttachment && depthAttachment->GetType() == FramebufferAttachmentType::Texture2DArray, "Invalid dir light shadow pass framebuffer. Texture2DArray depth attachment required");

			RenderCommand::SetViewport({ 0, 0 }, shadowResolution);
			
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
			FramebufferTexture2DArrayAttachment* depthArrayAttachment = static_cast<FramebufferTexture2DArrayAttachment*>(depthAttachment.get());
			for (int i = 0; i < activeLights; ++i)
			{
				Entity entity = dirLights[i];
				ComponentTransform& transform = entity.GetComponent<ComponentTransform>();
				ComponentDirectionalLight& light = entity.GetComponent<ComponentDirectionalLight>();

				if (light.CastShadows)
				{
					depthArrayAttachment->SetTargetIndex(i);
					targetFramebuffer->SetDepthAttachment(depthAttachment);
					RenderCommand::Clear();

					glm::mat3 rotationMatrix = glm::mat3(transform.GetTransform());

					// Remove scaling
					rotationMatrix[0] = glm::normalize(rotationMatrix[0]);
					rotationMatrix[1] = glm::normalize(rotationMatrix[1]);
					rotationMatrix[2] = glm::normalize(rotationMatrix[2]);

					glm::vec3 direction = glm::normalize(rotationMatrix * glm::vec3(0.0f, 0.0f, 1.0f));

					float shadowSize = light.ShadowMapProjectionSize;
					float nearClip = light.ShadowMapNearClip;
					float farClip = light.ShadowMapFarClip;

					glm::mat4 cameraTransform = glm::inverse(glm::lookAt(-direction * light.ShadowMapCameraDistance, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
					SceneCamera cam = SceneCamera(SCENE_CAMERA_ORTHOGRAPHIC);
					cam.SetOrthographic(shadowSize, (float)shadowResolution.x / (float)shadowResolution.y, nearClip, farClip);

					Renderer::BeginScene(cam, cameraTransform);
					{
						PE_PROFILE_SCOPE("Submit Mesh");
						AssetHandle shadowmapHandle = shadowmapMaterialInput->MaterialHandle;
						auto meshView = sceneContext->View<ComponentTransform, ComponentMeshRenderer>();
						for (auto entityID : meshView)
						{
							auto [meshTransform, mesh] = meshView.get<ComponentTransform, ComponentMeshRenderer>(entityID);
							BlendState blend;
							blend.Enabled = false;
							Renderer::SubmitMesh(mesh.MeshHandle, shadowmapHandle, meshTransform.GetTransform(), mesh.DepthState, mesh.CullState, blend, (int)entityID);
						}
					}
					Renderer::EndScene();
				}
			}
		};
		
		Ref<EditorAssetManager> assetManager = Project::GetActive()->GetEditorAssetManager();
		std::filesystem::path engineAssetsRelativeToProjectAssets = std::filesystem::path("assets").lexically_relative(Project::GetAssetDirectory());

		uint32_t shadowWidth = 1280; uint32_t shadowHeight = 1280;

		TextureSpecification depthSpec;
		depthSpec.Border = glm::vec4(1.0f);
		depthSpec.Width = shadowWidth;
		depthSpec.Height = shadowHeight;
		depthSpec.MinFilter = ImageMinFilter::NEAREST;
		depthSpec.MagFilter = ImageMagFilter::NEAREST;
		depthSpec.Wrap_S = ImageWrap::CLAMP_TO_BORDER;
		depthSpec.Wrap_T = ImageWrap::CLAMP_TO_BORDER;
		depthSpec.Wrap_R = ImageWrap::CLAMP_TO_BORDER;
		depthSpec.Format = ImageFormat::Depth32;

		if (!out_Framerenderer->ContainsRenderResource("DirLightShadowMap"))
		{
			Ref<Texture2DArray> dirLightShadowArray = AssetManager::CreateAsset<Texture2DArray>(true, depthSpec, std::vector<Buffer>(Renderer::MAX_ACTIVE_DIR_LIGHTS));
			out_Framerenderer->AddRenderResource<RenderComponentTexture>("DirLightShadowMap", false, dirLightShadowArray->Handle);
		}

		if (!out_Framerenderer->ContainsRenderResource("ShadowResolution"))
		{
			// In the future, this should be serialized. But many of the existing resources such as shadow framebuffers and textures aren't suited for this value to change during runtime
			glm::uvec2 shadowRes = { shadowWidth, shadowHeight };
			out_Framerenderer->AddRenderResource<RenderComponentPrimitiveType<glm::uvec2>>("ShadowResolution", false, shadowRes);
		}

		if (!out_Framerenderer->ContainsRenderResource("DirLightFramebuffer"))
		{
			FramebufferSpecification shadowFBOSpec;
			shadowFBOSpec.Width = shadowWidth;
			shadowFBOSpec.Height = shadowHeight;
			shadowFBOSpec.Samples = 1;

			Ref<FramebufferTexture2DArrayAttachment> dirLightShadowDepthArrayAttach = FramebufferTexture2DArrayAttachment::Create(FramebufferAttachmentPoint::Depth, dirLightShadowArray->Handle);
			Ref<Framebuffer> dirLightShadowsFramebuffer = Framebuffer::Create(shadowFBOSpec, {}, dirLightShadowDepthArrayAttach);

			out_Framerenderer->AddRenderResource<RenderComponentFramebuffer>("DirLightFramebuffer", false, dirLightShadowsFramebuffer);
		}

		if (!out_Framerenderer->ContainsRenderResource("ShadowmapMaterial"))
		{
			AssetHandle shadowmapShaderHandle = assetManager->ImportAssetFromFile(engineAssetsRelativeToProjectAssets / "shaders/DepthShader.glsl", true);
			Ref<Material> shadowmapMaterial = AssetManager::CreateAsset<Material>(true, shadowmapShaderHandle);
			out_Framerenderer->AddRenderResource<RenderComponentMaterial>("ShadowmapMaterial", false, shadowmapMaterial->Handle);
		}

		return { { RenderComponentType::PrimitiveType, RenderComponentType::Material },
				 { "ShadowResolution", "ShadowmapMaterial" },
				 dirLightShadowPassFunc };
	}

	Ref<ProbeBakeRenderer> RenderBuilder::InitProbeRenderer()
	{
		// Main render pass should create a view into the scene ReflectionProbeComponent pool
		
		// Each component has an environment map handle.
		// If the handle is invalid, generate an empty environment map asset
		
		// Begin bake iterations
		// Step through each probe, bind the base cubemap to the framebuffer -> render scene -> post process env map (filtering, irradiance)
		
		// Scene render should include previous iteration probes, skybox, lighting
		// Should not include bloom or tonemapping

		// Repeat process until all reflection probe components have reached their target bake iterations value

		Ref<ProbeBakeRenderer> probeRenderer = CreateRef<ProbeBakeRenderer>();
		Ref<FrameRenderer> renderer = probeRenderer->Renderer;

		glm::uvec2 cubemapRes = { probeRenderer->CubemapWidth, probeRenderer->CubemapHeight };
		renderer->AddRenderResource<RenderComponentPrimitiveType<glm::uvec2>>("CubemapRes", false, cubemapRes);

		renderer->AddRenderResource<RenderComponentEnvironmentMap>("TargetEnvironmentMap", false, 0);

		TextureSpecification depthSpec;
		depthSpec.Width = probeRenderer->CubemapWidth;
		depthSpec.Height = probeRenderer->CubemapHeight;
		depthSpec.GenerateMips = false;
		depthSpec.Format = ImageFormat::Depth24Stencil8;
		depthSpec.MinFilter = ImageMinFilter::NEAREST;
		depthSpec.MagFilter = ImageMagFilter::NEAREST;
		depthSpec.Wrap_S = ImageWrap::CLAMP_TO_BORDER;
		depthSpec.Wrap_T = ImageWrap::CLAMP_TO_BORDER;
		depthSpec.Wrap_R = ImageWrap::CLAMP_TO_BORDER;

		Ref<Texture2D> depthMap = AssetManager::CreateAsset<Texture2D>(true, depthSpec);

		FramebufferSpecification fboSpec;
		fboSpec.Width = probeRenderer->CubemapWidth;
		fboSpec.Height = probeRenderer->CubemapHeight;
		fboSpec.Samples = 1;

		Ref<FramebufferTexture2DAttachment> depthAttach = FramebufferTexture2DAttachment::Create(FramebufferAttachmentPoint::DepthStencil, depthMap->Handle);
		Ref<Framebuffer> mainFramebuffer = Framebuffer::Create(fboSpec, {}, depthAttach);

		renderer->AddRenderResource<RenderComponentFramebuffer>("MainFramebuffer", false, mainFramebuffer);

		Ref<EditorAssetManager> assetManager = Project::GetActive()->GetEditorAssetManager();
		std::filesystem::path engineAssetsRelativeToProjectAssets = std::filesystem::path("assets").lexically_relative(Project::GetAssetDirectory());

		AssetHandle skyboxShaderHandle = assetManager->ImportAssetFromFile(engineAssetsRelativeToProjectAssets / "shaders/Skybox.glsl", true);
		Ref<Material> skyboxMaterial = AssetManager::CreateAsset<Material>(true, skyboxShaderHandle);

		renderer->AddRenderResource<RenderComponentMaterial>("SkyboxMaterial", false, skyboxMaterial->Handle);

		FrameRenderer::OnEventFunc eventFunc = [](Event& e, FrameRenderer* self)
		{
			EventDispatcher dispatcher = EventDispatcher(e);
			dispatcher.DispatchEvent<MainViewportResizeEvent>([self](MainViewportResizeEvent& e)->bool {
				glm::uvec2 viewportSize = glm::uvec2(e.GetWidth(), e.GetHeight());
				self->GetRenderResource<RenderComponentPrimitiveType<glm::uvec2>>("CubemapRes")->Data = viewportSize;
				self->GetRenderResource<RenderComponentFramebuffer>("MainFramebuffer")->Framebuffer->Resize(viewportSize.x, viewportSize.y);
				return false;
			});
		};
		renderer->SetEventFunc(eventFunc);

		std::vector<RenderComponentType> forward3DInputSpec = { RenderComponentType::PrimitiveType, RenderComponentType::EnvironmentMap, RenderComponentType::Material };
		std::vector<std::string> forward3DInputBindings = { "CubemapRes", "TargetEnvironmentMap", "SkyboxMaterial" };
		RenderPass::OnRenderFunc forward3DPass = [](RenderPass::RenderPassContext& context, Ref<Framebuffer> targetFramebuffer, std::vector<IRenderComponent*> inputs) {
			PE_PROFILE_SCOPE("Scene 3D Render Pass");
			Ref<Scene>& sceneContext = context.ActiveScene;
			Ref<Camera> activeCamera = context.ActiveCamera;
			const glm::mat4& cameraWorldTransform = context.CameraWorldTransform;
			PE_CORE_ASSERT(inputs[0], "CubemapRes input required");
			PE_CORE_ASSERT(inputs[1], "TargetEnvironmentMap input required");
			PE_CORE_ASSERT(inputs[2], "Skybox material input required");
			RenderComponentPrimitiveType<glm::uvec2>* cubemapResInput = dynamic_cast<RenderComponentPrimitiveType<glm::uvec2>*>(inputs[0]);
			RenderComponentEnvironmentMap* targetEnvironmentMapInput = dynamic_cast<RenderComponentEnvironmentMap*>(inputs[1]);
			RenderComponentMaterial* skyboxMaterialInput = dynamic_cast<RenderComponentMaterial*>(inputs[2]);
			glm::uvec2 cubemapRes = cubemapResInput->Data;
			glm::uvec2 shadowRes = { 1024, 1024 };

			// Attach target to main framebuffer
			Ref<EnvironmentMap> targetEnvironmentMap = AssetManager::GetAsset<EnvironmentMap>(targetEnvironmentMapInput->EnvironmentHandle);
			PE_CORE_ASSERT(targetEnvironmentMap, "Error retrieving target environment map");

			Ref<TextureCubemap> baseCubemap = AssetManager::GetAsset<TextureCubemap>(targetEnvironmentMap->GetUnfilteredHandle());
			PE_CORE_ASSERT(baseCubemap, "Error retrieving base cubemap from environment map");
			if (baseCubemap->GetWidth() != cubemapRes.x || baseCubemap->GetHeight() != cubemapRes.y) { baseCubemap->Resize(cubemapRes.x, cubemapRes.y); }

			Ref<Material> skyboxMaterial = AssetManager::GetAsset<Material>(skyboxMaterialInput->MaterialHandle);
			auto skyboxParam = skyboxMaterial->GetParameter<SamplerCubeShaderParameterTypeStorage>("Skybox");

			Ref<FramebufferTextureCubemapAttachment> colourAttach = FramebufferTextureCubemapAttachment::Create(FramebufferAttachmentPoint::Colour0, targetEnvironmentMap->GetUnfilteredHandle());
			colourAttach->BindAsLayered = true;
			colourAttach->SetTargetFace(CubemapFace::POSITIVE_X);
			targetFramebuffer->AddColourAttachment(colourAttach);

			RenderCommand::SetViewport({ 0, 0 }, (glm::ivec2)cubemapRes);

			glm::vec3 viewPos = cameraWorldTransform[3];

			glm::mat4 viewMatrices[6] = {
				glm::lookAt(viewPos, viewPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
				glm::lookAt(viewPos, viewPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
				glm::lookAt(viewPos, viewPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
				glm::lookAt(viewPos, viewPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
				glm::lookAt(viewPos, viewPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
				glm::lookAt(viewPos, viewPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
			};

			for (uint8_t face = 0; face < 6; face++)
			{
				colourAttach->SetTargetFace((CubemapFace)face);
				colourAttach->BindToFramebuffer(targetFramebuffer.get());
				RenderCommand::Clear();

				Renderer::BeginScene(activeCamera->GetProjection(), glm::inverse(viewMatrices[face]), activeCamera->GetGamma(), activeCamera->GetExposure());

				{
					PE_PROFILE_SCOPE("Submit Mesh");
					auto view = sceneContext->View<ComponentTransform, ComponentMeshRenderer, ForwardCompatibleMaterialTag>();
					for (auto entityID : view) {
						auto [transform, mesh] = view.get<ComponentTransform, ComponentMeshRenderer>(entityID);

						AssetHandle materialHandle = mesh.MaterialHandle();
						Ref<Material> material = AssetManager::GetAsset<Material>(materialHandle);

						if (!material) { materialHandle = Renderer::GetDefaultMaterial(); } // Render with default forward material

						Renderer::SubmitMesh(mesh.MeshHandle, materialHandle, transform.GetTransform(), mesh.DepthState, mesh.CullState, BlendState(), (int)entityID);
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
							float aspectRatio = (float)shadowRes.x / (float)shadowRes.y;
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
							glm::vec4 position = glm::vec4(transform.WorldPosition(), 1.0f);
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

							glm::vec3 position = transform.WorldPosition();
							glm::vec3 direction = rotationMatrix * glm::vec3(0.0f, 0.0f, -1.0f);

							Renderer::SpotLight lightSource;
							lightSource.Position = glm::vec4(position, light.Range);
							lightSource.Direction = glm::vec4(direction, glm::cos(glm::radians(light.InnerCutoff)));
							lightSource.Diffuse = glm::vec4(light.Diffuse, 1.0f);
							lightSource.Specular = glm::vec4(light.Specular, 1.0f);
							lightSource.Ambient = glm::vec4(light.Ambient, glm::cos(glm::radians(light.OuterCutoff)));
							lightSource.ShadowData = glm::vec4((bool)light.CastShadows, light.ShadowMinBias, light.ShadowMaxBias, 1.0f);

							glm::mat4 lightView = glm::lookAt(position, position + direction, glm::vec3(0.0f, 1.0f, 0.0f));
							glm::mat4 projection = glm::perspective(glm::radians(90.0f), (float)shadowRes.x / (float)shadowRes.y, light.ShadowMapNearClip, light.ShadowMapFarClip);
							lightSource.LightMatrix = projection * lightView;

							Renderer::SubmitSpotLightSource(lightSource);
						}
					}
				}

				/*
				if (dirLightShadowInput) {
					Ref<Texture2DArray> dirLightShadowTexture = AssetManager::GetAsset<Texture2DArray>(dirLightShadowInput->TextureHandle);
					PE_CORE_ASSERT(dirLightShadowTexture->GetType() == AssetType::Texture2DArray, "Invalid directional light shadow map type");
					dirLightShadowTexture->Bind(0);
				}
				if (spotLightShadowInput) {
					Ref<Texture2DArray> spotLightShadowTexture = AssetManager::GetAsset<Texture2DArray>(spotLightShadowInput->TextureHandle);
					PE_CORE_ASSERT(spotLightShadowTexture->GetType() == AssetType::Texture2DArray, "Invalid spot light shadow map type");
					spotLightShadowTexture->Bind(1);
				}
				if (pointLightShadowInput) {
					Ref<TextureCubemapArray> pointLightShadowTexture = AssetManager::GetAsset<TextureCubemapArray>(pointLightShadowInput->TextureHandle);
					PE_CORE_ASSERT(pointLightShadowTexture->GetType() == AssetType::TextureCubemapArray, "Invalid point light shadow map type");
					pointLightShadowTexture->Bind(2);
				}

				// TODO: Only use environment map reflections after first pass as it needs to blend with other reflection probes in the scene
				if (envMapInput)
				{
					Ref<EnvironmentMap> envMap = AssetManager::GetAsset<EnvironmentMap>(envMapInput->EnvironmentHandle);
					AssetManager::GetAsset<TextureCubemap>(envMap->GetIrradianceMapHandle())->Bind(10);
					AssetManager::GetAsset<TextureCubemap>(envMap->GetPrefilteredMapHandle())->Bind(11);
					AssetManager::GetAsset<Texture2D>(EnvironmentMap::GetBRDFLutHandle())->Bind(12);
				}
				*/

				Renderer::EndScene();
			
				// Draw sky
				{
					auto view = sceneContext->View<ComponentRenderVolume>();
					for (auto entityID : view)
					{
						// Use first volume since we do not currently support local volumes
						auto renderVolumeComponent = view.get<ComponentRenderVolume>(entityID);

						switch (renderVolumeComponent.SkyboxType)
						{
						case ComponentRenderVolume::SkyType::SKY_NONE:
							skyboxParam->TextureHandle = 0;
							return;
						case ComponentRenderVolume::SkyType::SKY_SKYBOX:
							skyboxParam->TextureHandle = renderVolumeComponent.SkyboxHandle;
							break;
						case ComponentRenderVolume::SkyType::SKY_ENVMAP:
						{
							Ref<EnvironmentMap> envMap = AssetManager::GetAsset<EnvironmentMap>(renderVolumeComponent.EnvironmentMapHandle);
							if (envMap) { skyboxParam->TextureHandle = envMap->GetUnfilteredHandle(); }
							break;
						}
						}

						break;
					}

					Renderer::BeginScene(activeCamera->GetProjection(), glm::inverse(viewMatrices[face]), activeCamera->GetGamma(), activeCamera->GetExposure());

					DepthState depthState;
					depthState.Func = DepthFunc::LEQUAL;
					FaceCulling cullState = FaceCulling::FRONT;

					Renderer::DrawDefaultCubeImmediate(skyboxMaterial, glm::mat4(1.0f), depthState, cullState, BlendState(), -1);

					Renderer::EndScene();
				}
			}

			baseCubemap->GenerateMipmaps();

			// Warning: Unbinds target framebuffer
			targetEnvironmentMap->ProcessBaseCubemap();
		};

		renderer->AddRenderPass(RenderPass(forward3DInputSpec, forward3DPass, "IBL_Bake_Forward_3D"), mainFramebuffer, forward3DInputBindings);

		return probeRenderer;
	}

	Ref<BasicSceneRenderer> RenderBuilder::InitBasicRenderer()
	{
		// shadow passes
		// forward 2d pass
		// forward 3d pass
		// skybox pass
		// bloom passes
		// tonemapping pass

		return Ref<BasicSceneRenderer>();
	}
}