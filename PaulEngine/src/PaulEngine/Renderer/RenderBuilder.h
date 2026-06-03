#pragma once
#include "PaulEngine/Core/Core.h"
#include "PaulEngine/Scene/Components.h"
#include "PaulEngine/Renderer/FrameRenderer.h"
#include "PaulEngine/Events/SceneEvent.h"
#include "PaulEngine/Renderer/Asset/EnvironmentMap.h"
#include "PaulEngine/Asset/AssetImporter.h"

namespace PaulEngine
{
	struct ProbeBakeRenderer
	{
		Ref<FrameRenderer> Renderer = CreateRef<FrameRenderer>(); // todo: why does this need to be on the heap?
		Ref<Scene> SceneContext = nullptr;
		uint32_t CubemapWidth = 1024;
		uint32_t CubemapHeight = 1024;

		void Run()
		{
			if (SceneContext)
			{
				auto envMapComponent = Renderer->GetRenderResource<RenderComponentEnvironmentMap>("TargetEnvironmentMap");
				if (envMapComponent) {
					auto resComponent = Renderer->GetRenderResource<RenderComponentPrimitiveType<glm::uvec2>>("CubemapRes");
					if (resComponent) { 
						glm::uvec2 newRes = { CubemapWidth, CubemapHeight };
						if (newRes != resComponent->Data)
						{
							MainViewportResizeEvent e = MainViewportResizeEvent(CubemapWidth, CubemapHeight);
							Renderer->OnEvent(e);
						}
					}

					Ref<SceneCamera> captureCamera = CreateRef<SceneCamera>(SCENE_CAMERA_PERSPECTIVE);
					captureCamera->SetViewportSize(CubemapWidth, CubemapHeight);

					auto probeView = SceneContext->View<ComponentTransform, ComponentReflectionProbe>();
					
					size_t numProbes = 0;
					// This is a bit silly, but entt::view only has size_hint which estimates the size of a view
					for (auto entityID : probeView)
					{
						numProbes++;
					}
					
					if (numProbes == 1) { PE_CORE_INFO("Running probe baking for 1 probe"); }
					else				{ PE_CORE_INFO("Running probe baking for {0} probes", numProbes); }

					size_t currentProbe = 1;
					for (auto entityID : probeView)
					{
						PE_CORE_INFO("    Baking probe {0}/{1}...", currentProbe++, numProbes);
						auto [transform, probe] = probeView.get(entityID);

						if (!AssetManager::IsAssetHandleValid(probe.GetEnvironmentMapHandle()))
						{
							probe.m_EnvironmentMapHandle = AssetManager::CreateAsset<EnvironmentMap>(false, glm::uvec2(CubemapWidth, CubemapHeight), false)->Handle;
						}
						envMapComponent->EnvironmentHandle = probe.GetEnvironmentMapHandle();
						Renderer->RenderFrame(SceneContext, captureCamera, transform.GetTransform());
					}
					PE_CORE_INFO("Reflection probe baking complete");

					// Save env maps to disk
					EnvironmentMap::ValidateProbeCacheDirectory();
					const std::filesystem::path cacheDirectory = EnvironmentMap::GetProbeCacheDirectory();
					
					currentProbe = 1;
					PE_CORE_INFO("Saving probes to disk");
					for (auto entityID : probeView)
					{
						PE_CORE_INFO("    Saving probe {0}/{1}...", currentProbe++, numProbes);
						auto [transform, probe] = probeView.get(entityID);

						AssetHandle envMapHandle = probe.GetEnvironmentMapHandle();
						Ref<EnvironmentMap> envMapAsset = AssetManager::GetAsset<EnvironmentMap>(envMapHandle);
						
						AssetHandle baseCubemapHandle = envMapAsset->GetUnfilteredHandle();
						AssetHandle irradianceCubemapHandle = envMapAsset->GetIrradianceMapHandle();
						AssetHandle prefilteredCubemapHandle = envMapAsset->GetPrefilteredMapHandle();

						EnvironmentMap::CacheCubemap(baseCubemapHandle, cacheDirectory / (std::to_string(baseCubemapHandle) + ".ccm"));
						EnvironmentMap::CacheCubemap(irradianceCubemapHandle, cacheDirectory / (std::to_string(irradianceCubemapHandle) + ".ccm"));
						const uint8_t maxMipLevels = 7;
						EnvironmentMap::CacheCubemap(prefilteredCubemapHandle, cacheDirectory / (std::to_string(prefilteredCubemapHandle) + ".ccm"), maxMipLevels);

						AssetImporter::SerializeAssetFromType(envMapAsset);
						std::filesystem::path relativePath = AssetImporter::GetAssetFileCachePath(envMapHandle).lexically_relative(Project::GetAssetDirectory());
						Project::GetActive()->GetEditorAssetManager()->UpdateAssetSourcePath(envMapHandle, relativePath);
					}
					PE_CORE_INFO("Reflection probe saving complete");
				} else { PE_CORE_ERROR("Missing target environment map render component"); }
			} else { PE_CORE_ERROR("Invalid scene context"); }
		}
	};

	struct BasicSceneRenderer
	{
		FrameRenderer Renderer = FrameRenderer();
		WeakRef<Scene> SceneContext;

		void Run()
		{
			if (Ref<Scene> scene = SceneContext.lock())
			{
				Entity cameraEntity = scene->GetPrimaryCameraEntity();
				Ref<SceneCamera> sceneCamera = (cameraEntity) ? CreateRef<SceneCamera>(cameraEntity.GetComponent<ComponentCamera>().Camera) : nullptr;
				glm::mat4 cameraWorldTransform = (cameraEntity) ? cameraEntity.GetComponent<ComponentTransform>().GetTransform() : glm::mat4(1.0f);
				Renderer.RenderFrame(scene, sceneCamera, cameraWorldTransform);
			}
		}
	};

	class RenderBuilder
	{
	public:
		static WeakRef<ProbeBakeRenderer> GetProbeBakerInstance()
		{
			static Ref<ProbeBakeRenderer> renderer = InitProbeRenderer();
			return CreateWeak<ProbeBakeRenderer>(renderer);
		}

		static WeakRef<BasicSceneRenderer> GetBasicSceneRenderer()
		{
			static Ref<BasicSceneRenderer> renderer = InitBasicRenderer();
			return CreateWeak<BasicSceneRenderer>(renderer);
		}

		static void BuildBasicForwardRenderer(Ref<FrameRenderer> out_Framerenderer);

		struct PrebuiltRenderFunc
		{
			std::vector<RenderComponentType> InputSpec;
			std::vector<const char*> DefaultInputBindings;
			RenderPass::OnRenderFunc Func;
		};
		// BuildXPass functions will return an OnRenderFunc object that can be added to your frame renderer 
		// instance and will also create all of the necessary resources inside of out_Framerenderer
		static PrebuiltRenderFunc BuildDirLightShadowPass(Ref<FrameRenderer> out_Framerenderer);

	private:
		static Ref<ProbeBakeRenderer> InitProbeRenderer();
		static Ref<BasicSceneRenderer> InitBasicRenderer();
	};
}