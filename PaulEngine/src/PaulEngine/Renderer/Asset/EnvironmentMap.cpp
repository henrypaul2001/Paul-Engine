#include "pepch.h"
#include "EnvironmentMap.h"

#include "PaulEngine/Asset/AssetManager.h"
#include "PaulEngine/Asset/TextureImporter.h"
#include "PaulEngine/Renderer/RenderCommand.h"
#include "PaulEngine/Renderer/Asset/Material.h"
#include "PaulEngine/Scene/SceneCamera.h"
#include "PaulEngine/Renderer/Renderer.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

constexpr uint32_t CUBE_MAP_RESOLUTION = 512u;
constexpr uint32_t IRRADIANCE_MAP_RESOLUTION = 32u;

namespace PaulEngine
{
	Ref<Framebuffer> EnvironmentMap::s_CubeCaptureFBO = nullptr;

	AssetHandle EnvironmentMap::s_ConvertToCubemapShaderHandle = 0;
	AssetHandle EnvironmentMap::s_ConvertToCubemapMaterialHandle = 0;

	AssetHandle EnvironmentMap::s_ConvolutionShaderHandle = 0;
	AssetHandle EnvironmentMap::s_ConvolutionMaterialHandle = 0;

	AssetHandle EnvironmentMap::s_PrefilterShaderHandle = 0;
	AssetHandle EnvironmentMap::s_PrefilterMaterialHandle = 0;

	AssetHandle EnvironmentMap::s_BRDFLutShaderHandle = 0;
	AssetHandle EnvironmentMap::s_BRDFLutMaterialHandle = 0;
	AssetHandle EnvironmentMap::s_BRDFLutTextureHandle = 0;

	EnvironmentMap::EnvironmentMap(const std::filesystem::path& hdrPath, bool persistentAsset)
	{
		PE_PROFILE_FUNCTION();
		PE_CORE_ASSERT(hdrPath.extension() == ".hdr", "Invalid file extension");

		TextureSpecification spec;
		spec.Format = ImageFormat::RGB32F;
		spec.Wrap_S = ImageWrap::CLAMP_TO_EDGE;
		spec.Wrap_T = ImageWrap::CLAMP_TO_EDGE;
		spec.MinFilter = ImageMinFilter::LINEAR;
		spec.MagFilter = ImageMagFilter::LINEAR;
		Ref<Texture2D> hdrTexture = TextureImporter::LoadTexture2D(hdrPath, spec);

		spec = TextureSpecification();
		spec.Format = ImageFormat::RGB32F;
		spec.Width = CUBE_MAP_RESOLUTION;
		spec.Height = CUBE_MAP_RESOLUTION;
		spec.Wrap_S = ImageWrap::CLAMP_TO_EDGE;
		spec.Wrap_T = ImageWrap::CLAMP_TO_EDGE;
		spec.Wrap_R = ImageWrap::CLAMP_TO_EDGE;
		spec.MinFilter = ImageMinFilter::LINEAR;
		spec.MagFilter = ImageMagFilter::LINEAR;
		spec.GenerateMips = true;
		
		Ref<TextureCubemap> baseCubemap = AssetManager::CreateAsset<TextureCubemap>(persistentAsset, spec);
		m_BaseCubemapHandle = baseCubemap->Handle;

		spec.MinFilter = ImageMinFilter::LINEAR_MIPMAP_LINEAR;
		m_PrefilteredCubemapHandle = AssetManager::CreateAsset<TextureCubemap>(persistentAsset, spec)->Handle;

		spec.Width = 32;
		spec.Height = 32;
		spec.GenerateMips = false;
		m_IrradianceCubemapHandle = AssetManager::CreateAsset<TextureCubemap>(persistentAsset, spec)->Handle;

		// Convert loaded equirectangular hdr texture to a cubemap texture
		ConvertEquirectangularToCubemap(hdrTexture, m_BaseCubemapHandle);

		ConvoluteEnvironmentMap(baseCubemap, m_IrradianceCubemapHandle);

		PrefilterEnvironmentMap(baseCubemap, m_PrefilteredCubemapHandle);

		if (!AssetManager::IsAssetHandleValid(s_BRDFLutTextureHandle)) { GenerateBRDFLut(); }
	}

	void EnvironmentMap::ConvertEquirectangularToCubemap(Ref<Texture2D> equirectangular, AssetHandle targetCubemapHandle)
	{
		PE_PROFILE_FUNCTION();
		if (s_CubeCaptureFBO == nullptr) { InitEnvMapProcessing(); }

		// Attach target cubemap to framebuffer draw attachment
		PE_CORE_ASSERT(AssetManager::IsAssetHandleValid(targetCubemapHandle), "Invalid target cubemap asset handle");
		Ref<FramebufferTextureCubemapAttachment> drawAttachment = FramebufferTextureCubemapAttachment::Create(FramebufferAttachmentPoint::Colour0, targetCubemapHandle);
		drawAttachment->BindAsLayered = false;
		s_CubeCaptureFBO->AddColourAttachment(drawAttachment);
		s_CubeCaptureFBO->Bind();

		// Resize depth buffer
		s_CubeCaptureFBO->Resize(CUBE_MAP_RESOLUTION, CUBE_MAP_RESOLUTION);

		RenderCommand::Clear();
		RenderCommand::SetViewport({ 0, 0 }, { CUBE_MAP_RESOLUTION, CUBE_MAP_RESOLUTION });

		// Setup cubemap capture view projections
		Ref<Material> cubeCaptureMaterial = AssetManager::GetAsset<Material>(s_ConvertToCubemapMaterialHandle);
		const glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		const glm::mat4 viewProjections[] = {
			captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
			captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
			captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
			captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
			captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
			captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
		};

		UniformBufferStorage* uboStorage = cubeCaptureMaterial->GetParameter<UBOShaderParameterTypeStorage>("CubeData")->UBO().get();
		uboStorage->SetLocalData("ViewProjections[0][0]", viewProjections[0]);
		uboStorage->SetLocalData("ViewProjections[0][1]", viewProjections[1]);
		uboStorage->SetLocalData("ViewProjections[0][2]", viewProjections[2]);
		uboStorage->SetLocalData("ViewProjections[0][3]", viewProjections[3]);
		uboStorage->SetLocalData("ViewProjections[0][4]", viewProjections[4]);
		uboStorage->SetLocalData("ViewProjections[0][5]", viewProjections[5]);
		uboStorage->SetLocalData("CubemapIndex", 0);

		// Render
		SceneCamera cam = SceneCamera(SCENE_CAMERA_PERSPECTIVE);
		cam.SetPerspective(90.0f, 1.0f, 0.1f, 10.0f);

		Renderer::BeginScene(cam, glm::mat4(1.0f));
		equirectangular->Bind(0);
		glm::mat4 transform = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f));
		Renderer::DrawDefaultCubeImmediate(cubeCaptureMaterial, transform, DepthState(), FaceCulling::NONE, BlendState(), -1);
		Renderer::EndScene();

		AssetManager::GetAsset<TextureCubemap>(targetCubemapHandle)->GenerateMipmaps();
		s_CubeCaptureFBO->Unbind();
	}

	void EnvironmentMap::ConvoluteEnvironmentMap(Ref<TextureCubemap> environmentMap, AssetHandle targetCubemapHandle)
	{
		PE_PROFILE_FUNCTION();
		if (s_CubeCaptureFBO == nullptr) { InitEnvMapProcessing(); }

		// Attach target cubemap to framebuffer draw attachment
		PE_CORE_ASSERT(AssetManager::IsAssetHandleValid(targetCubemapHandle), "Invalid target cubemap asset handle");
		Ref<FramebufferTextureCubemapAttachment> drawAttachment = FramebufferTextureCubemapAttachment::Create(FramebufferAttachmentPoint::Colour0, targetCubemapHandle);
		drawAttachment->BindAsLayered = false;
		s_CubeCaptureFBO->AddColourAttachment(drawAttachment);
		s_CubeCaptureFBO->Bind();

		// Resize depth buffer
		s_CubeCaptureFBO->Resize(IRRADIANCE_MAP_RESOLUTION, IRRADIANCE_MAP_RESOLUTION);

		RenderCommand::Clear();
		RenderCommand::SetViewport({ 0, 0 }, { IRRADIANCE_MAP_RESOLUTION, IRRADIANCE_MAP_RESOLUTION });

		// TODO: would it be worth just hardcoding these matrices into the shader asset? They never change.
		// Setup cubemap capture view projections
		Ref<Material> convolutionMaterial = AssetManager::GetAsset<Material>(s_ConvolutionMaterialHandle);
		const glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		const glm::mat4 viewProjections[] = {
			captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
			captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
			captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
			captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
			captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
			captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
		};

		UniformBufferStorage* uboStorage = convolutionMaterial->GetParameter<UBOShaderParameterTypeStorage>("CubeData")->UBO().get();
		uboStorage->SetLocalData("ViewProjections[0][0]", viewProjections[0]);
		uboStorage->SetLocalData("ViewProjections[0][1]", viewProjections[1]);
		uboStorage->SetLocalData("ViewProjections[0][2]", viewProjections[2]);
		uboStorage->SetLocalData("ViewProjections[0][3]", viewProjections[3]);
		uboStorage->SetLocalData("ViewProjections[0][4]", viewProjections[4]);
		uboStorage->SetLocalData("ViewProjections[0][5]", viewProjections[5]);
		uboStorage->SetLocalData("CubemapIndex", 0);

		// Render
		SceneCamera cam = SceneCamera(SCENE_CAMERA_PERSPECTIVE);
		cam.SetPerspective(90.0f, 1.0f, 0.1f, 10.0f);

		Renderer::BeginScene(cam, glm::mat4(1.0f));
		environmentMap->Bind(0);
		glm::mat4 transform = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f));
		Renderer::DrawDefaultCubeImmediate(convolutionMaterial, transform, DepthState(), FaceCulling::NONE, BlendState(), -1);
		Renderer::EndScene();
		s_CubeCaptureFBO->Unbind();
	}

	void EnvironmentMap::PrefilterEnvironmentMap(Ref<TextureCubemap> environmentMap, AssetHandle targetCubemapHandle)
	{
		PE_PROFILE_FUNCTION();
		if (s_CubeCaptureFBO == nullptr) { InitEnvMapProcessing(); }

		// Attach target cubemap to framebuffer draw attachment
		PE_CORE_ASSERT(AssetManager::IsAssetHandleValid(targetCubemapHandle), "Invalid target cubemap asset handle");
		Ref<FramebufferTextureCubemapAttachment> drawAttachment = FramebufferTextureCubemapAttachment::Create(FramebufferAttachmentPoint::Colour0, targetCubemapHandle);
		drawAttachment->BindAsLayered = false;
		s_CubeCaptureFBO->AddColourAttachment(drawAttachment);
		s_CubeCaptureFBO->Bind();

		// Resize depth buffer
		s_CubeCaptureFBO->Resize(CUBE_MAP_RESOLUTION, CUBE_MAP_RESOLUTION);

		RenderCommand::Clear();
		RenderCommand::SetViewport({ 0, 0 }, { CUBE_MAP_RESOLUTION, CUBE_MAP_RESOLUTION });

		// Setup cubemap capture view projections
		Ref<Material> prefilterMaterial = AssetManager::GetAsset<Material>(s_PrefilterMaterialHandle);
		const glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		const glm::mat4 viewProjections[] = {
			captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
			captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
			captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
			captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
			captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
			captureProjection * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
		};

		UniformBufferStorage* uboStorage = prefilterMaterial->GetParameter<UBOShaderParameterTypeStorage>("CubeData")->UBO().get();
		uboStorage->SetLocalData("ViewProjections[0][0]", viewProjections[0]);
		uboStorage->SetLocalData("ViewProjections[0][1]", viewProjections[1]);
		uboStorage->SetLocalData("ViewProjections[0][2]", viewProjections[2]);
		uboStorage->SetLocalData("ViewProjections[0][3]", viewProjections[3]);
		uboStorage->SetLocalData("ViewProjections[0][4]", viewProjections[4]);
		uboStorage->SetLocalData("ViewProjections[0][5]", viewProjections[5]);
		uboStorage->SetLocalData("CubemapIndex", 0);

		UniformBufferStorage* prefilterParams = prefilterMaterial->GetParameter<UBOShaderParameterTypeStorage>("PrefilterParams")->UBO().get();
		prefilterParams->SetLocalData("FaceWidth", CUBE_MAP_RESOLUTION);
		prefilterParams->SetLocalData("FaceHeight", CUBE_MAP_RESOLUTION);

		// Render
		SceneCamera cam = SceneCamera(SCENE_CAMERA_PERSPECTIVE);
		cam.SetPerspective(90.0f, 1.0f, 0.1f, 10.0f);

		const uint8_t maxMipLevels = 7;
		for (uint8_t mip = 0; mip < maxMipLevels; mip++)
		{
			const uint32_t mipWidth = CUBE_MAP_RESOLUTION * std::pow(0.5f, mip);
			const uint32_t mipHeight = mipWidth;

			Ref<FramebufferAttachment> colourAttach = s_CubeCaptureFBO->GetAttachment(FramebufferAttachmentPoint::Colour0);
			dynamic_cast<FramebufferTextureCubemapAttachment*>(colourAttach.get())->TargetMipLevel = mip;
			
			// Re-bind attachment
			colourAttach->BindToFramebuffer(s_CubeCaptureFBO.get());
			s_CubeCaptureFBO->Bind();

			// Resize depth buffer
			s_CubeCaptureFBO->GetDepthAttachment()->Resize(mipWidth, mipHeight);
			RenderCommand::ClearDepth();

			RenderCommand::SetViewport({ 0, 0 }, { mipWidth, mipHeight });

			float roughness = (float)mip / (float)(maxMipLevels - 1);

			prefilterParams->SetLocalData("Roughness", roughness);

			Renderer::BeginScene(cam, glm::mat4(1.0f));
			environmentMap->Bind(0);
			glm::mat4 transform = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f));
			Renderer::DrawDefaultCubeImmediate(prefilterMaterial, transform, DepthState(), FaceCulling::NONE, BlendState(), -1);
			Renderer::EndScene();
		}
		s_CubeCaptureFBO->Unbind();
	}

	void EnvironmentMap::InitEnvMapProcessing()
	{
		PE_PROFILE_FUNCTION();

		TextureSpecification depthSpec;
		depthSpec.Format = ImageFormat::Depth32;
		depthSpec.GenerateMips = false;
		depthSpec.MinFilter = ImageMinFilter::NEAREST;
		depthSpec.MagFilter = ImageMagFilter::NEAREST;
		depthSpec.Wrap_S = ImageWrap::CLAMP_TO_EDGE;
		depthSpec.Wrap_T = ImageWrap::CLAMP_TO_EDGE;
		depthSpec.Wrap_R = ImageWrap::CLAMP_TO_EDGE;
		depthSpec.Width = CUBE_MAP_RESOLUTION;
		depthSpec.Height = CUBE_MAP_RESOLUTION;
		depthSpec.Border = glm::vec4(1.0f);

		Ref<TextureCubemap> depthTexture = AssetManager::CreateAsset<TextureCubemap>(true, depthSpec);

		FramebufferSpecification spec;
		spec.Samples = 1;
		spec.Width = CUBE_MAP_RESOLUTION;
		spec.Height = CUBE_MAP_RESOLUTION;

		// TODO: change depth attachment to a render buffer object when they are implemented
		Ref<FramebufferTextureCubemapAttachment> depth = FramebufferTextureCubemapAttachment::Create(FramebufferAttachmentPoint::Depth, depthTexture->Handle);
		depth->BindAsLayered = false;
		s_CubeCaptureFBO = Framebuffer::Create(spec, {}, depth);

		// Load material
		Ref<EditorAssetManager> assetManager = Project::GetActive()->GetEditorAssetManager();
		std::filesystem::path engineAssetsRelativeToProjectAssets = std::filesystem::path("assets").lexically_relative(Project::GetAssetDirectory());

		s_ConvertToCubemapShaderHandle = assetManager->ImportAssetFromFile(engineAssetsRelativeToProjectAssets / "shaders/EquirectangularToCubemap.glsl", false);
		s_ConvertToCubemapMaterialHandle = AssetManager::CreateAsset<Material>(true, s_ConvertToCubemapShaderHandle)->Handle;
	
		s_ConvolutionShaderHandle = assetManager->ImportAssetFromFile(engineAssetsRelativeToProjectAssets / "shaders/CubemapConvolution.glsl", false);
		s_ConvolutionMaterialHandle = AssetManager::CreateAsset<Material>(true, s_ConvolutionShaderHandle)->Handle;

		s_PrefilterShaderHandle = assetManager->ImportAssetFromFile(engineAssetsRelativeToProjectAssets / "shaders/CubemapPrefilter.glsl", false);
		s_PrefilterMaterialHandle = AssetManager::CreateAsset<Material>(true, s_PrefilterShaderHandle)->Handle;
	}

	void EnvironmentMap::GenerateBRDFLut()
	{
		PE_PROFILE_FUNCTION();
		Ref<EditorAssetManager> assetManager = Project::GetActive()->GetEditorAssetManager();
		std::filesystem::path engineAssetsRelativeToProjectAssets = std::filesystem::path("assets").lexically_relative(Project::GetAssetDirectory());

		s_BRDFLutShaderHandle = assetManager->ImportAssetFromFile(engineAssetsRelativeToProjectAssets / "shaders/BRDFLut.glsl", false);
		s_BRDFLutMaterialHandle = AssetManager::CreateAsset<Material>(false, s_BRDFLutShaderHandle)->Handle;

		TextureSpecification brdfSpec;
		brdfSpec.Format = ImageFormat::RG16F;
		brdfSpec.Width = CUBE_MAP_RESOLUTION;
		brdfSpec.Height = CUBE_MAP_RESOLUTION;
		brdfSpec.Wrap_S = ImageWrap::CLAMP_TO_EDGE;
		brdfSpec.Wrap_T = ImageWrap::CLAMP_TO_EDGE;
		brdfSpec.MinFilter = ImageMinFilter::LINEAR;
		brdfSpec.MagFilter = ImageMagFilter::LINEAR;
		brdfSpec.GenerateMips = false;
		s_BRDFLutTextureHandle = AssetManager::CreateAsset<Texture2D>(true, brdfSpec)->Handle;

		TextureSpecification depthSpec;
		depthSpec.Format = ImageFormat::Depth32;
		depthSpec.GenerateMips = false;
		depthSpec.MinFilter = ImageMinFilter::NEAREST;
		depthSpec.MagFilter = ImageMagFilter::NEAREST;
		depthSpec.Wrap_S = ImageWrap::CLAMP_TO_EDGE;
		depthSpec.Wrap_T = ImageWrap::CLAMP_TO_EDGE;
		depthSpec.Wrap_R = ImageWrap::CLAMP_TO_EDGE;
		depthSpec.Width = CUBE_MAP_RESOLUTION;
		depthSpec.Height = CUBE_MAP_RESOLUTION;
		depthSpec.Border = glm::vec4(1.0f);
		Ref<Texture2D> depthTexture = AssetManager::CreateAsset<Texture2D>(false, depthSpec);

		FramebufferSpecification spec;
		spec.Samples = 1;
		spec.Width = CUBE_MAP_RESOLUTION;
		spec.Height = CUBE_MAP_RESOLUTION;

		Ref<FramebufferTexture2DAttachment> brdfAttach = FramebufferTexture2DAttachment::Create(FramebufferAttachmentPoint::Colour0, s_BRDFLutTextureHandle);
		Ref<FramebufferTexture2DAttachment> depthAttach = FramebufferTexture2DAttachment::Create(FramebufferAttachmentPoint::Depth, depthTexture->Handle);

		Ref<Framebuffer> tempFBO = Framebuffer::Create(spec, { brdfAttach }, depthAttach);

		tempFBO->Bind();
		RenderCommand::SetViewport({ 0, 0 }, { CUBE_MAP_RESOLUTION, CUBE_MAP_RESOLUTION });
		RenderCommand::Clear();

		Ref<Material> brdfLutMaterial = AssetManager::GetAsset<Material>(s_BRDFLutMaterialHandle);

		Renderer::BeginScene(SceneCamera(), glm::mat4(1.0f));
		BlendState blend;
		blend.Enabled = false;
		Renderer::DrawDefaultQuadImmediate(brdfLutMaterial, glm::mat4(1.0f), DepthState(), FaceCulling::NONE, blend, -1);
		Renderer::EndScene();

		tempFBO->Unbind();
	}
}