#include "ResourceManager.h"
#include "RenderManager.h"
#include "AudioManager.h"
#include "ASSIMPModelLoader.h"
namespace Engine {

	ResourceManager* ResourceManager::instance = nullptr;
	ResourceManager::ResourceManager()
	{
		ResetModelLoaderTextureTranslationsToDefault();

		textureSlotLookup = {
			{ "dirLight.ShadowMap", 0 },
			{ "spotlightShadowAtlas", 1 },
			{ "pointLightShadowArray", 2},

			{ "material.TEXTURE_ALBEDO1", 3 },
			{ "material.TEXTURE_DIFFUSE1", 3 },
			{ "material.TEXTURE_NORMAL1", 4 },
			{ "material.TEXTURE_METALLIC1", 5 },
			{ "material.TEXTURE_SPECULAR1", 5 },
			{ "material.TEXTURE_ROUGHNESS1", 6 },
			{ "material.TEXTURE_AO1", 7 },
			{ "material.TEXTURE_DISPLACE1", 8 },
			{ "material.TEXTURE_OPACITY1", 9 },

			{ "gPosition", 3 },
			{ "gNormal", 4 },
			{ "gAlbedo", 5 },
			{ "gSpecular", 6 },
			{ "gArm", 6 },
			{ "gPBRFLAG", 7 },
			{ "SSAO", 8 },

			{ "globalIBL.irradianceMap", 10 },
			{ "globalIBL.prefilterMap", 11 },
			{ "brdfLUT", 12 },

			{ "localIBLIrradianceMapArray", 13 },
			{ "localIBLPrefilterMapArray", 14 },
		};

		// Setup freetype
		if (FT_Init_FreeType(&freetypeLib)) {
			std::cout << "FAIL::RESOURCEMANAGER::Failed to initialize FreeType Library" << std::endl;
		}

		// Load defaults
		defaultMaterial = new Material();
		defaultMaterial->baseColour = glm::vec3(0.8f, 0.8f, 0.8f);
		defaultMaterial->specular = glm::vec3(1.0f, 1.0f, 1.0f);
		defaultMaterial->shininess = 105.0f;
		AddMaterial("Default Material", defaultMaterial, true);
		//defaultMaterial->shininess = 60.0f;

		defaultMaterialPBR = new PBRMaterial();
		defaultMaterialPBR->baseColour = glm::vec3(1.0f, 1.0f, 1.0f);
		defaultMaterialPBR->metallic = 0.0f;
		defaultMaterialPBR->roughness = 0.5f;
		defaultMaterialPBR->ao = 1.0f;
		AddMaterial("Default Material PBR", defaultMaterialPBR, true);

		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		Vertex vertex;
#pragma region defaultCube
		// Back face
		vertex.Position = glm::vec3(-1.0f, -1.0f, -1.0f);
		vertex.Normal = glm::vec3(0.0f, 0.0f, -1.0f);
		vertex.TexCoords = glm::vec2(0.0f, 0.0f);
		vertices.push_back(vertex); // bottom left

		vertex.Position = glm::vec3(1.0f, 1.0f, -1.0f);
		vertex.Normal = glm::vec3(0.0f, 0.0f, -1.0f);
		vertex.TexCoords = glm::vec2(1.0f, 1.0f);
		vertices.push_back(vertex); // top right

		vertex.Position = glm::vec3(1.0f, -1.0f, -1.0f);
		vertex.Normal = glm::vec3(0.0f, 0.0f, -1.0f);
		vertex.TexCoords = glm::vec2(1.0f, 0.0f);
		vertices.push_back(vertex); // bottom right

		vertex.Position = glm::vec3(-1.0f, 1.0f, -1.0f);
		vertex.Normal = glm::vec3(0.0f, 0.0f, -1.0f);
		vertex.TexCoords = glm::vec2(0.0f, 1.0f);
		vertices.push_back(vertex); // top left

		// Front face
		vertex.Position = glm::vec3(-1.0f, -1.0f, 1.0f);
		vertex.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
		vertex.TexCoords = glm::vec2(0.0f, 0.0f);
		vertices.push_back(vertex); // bottom left

		vertex.Position = glm::vec3(1.0f, -1.0f, 1.0f);
		vertex.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
		vertex.TexCoords = glm::vec2(1.0f, 0.0f);
		vertices.push_back(vertex); // bottom right

		vertex.Position = glm::vec3(1.0f, 1.0f, 1.0f);
		vertex.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
		vertex.TexCoords = glm::vec2(1.0f, 1.0f);
		vertices.push_back(vertex); // top right

		vertex.Position = glm::vec3(-1.0f, 1.0f, 1.0f);
		vertex.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
		vertex.TexCoords = glm::vec2(0.0f, 1.0f);
		vertices.push_back(vertex); // top left

		// Left face
		vertex.Position = glm::vec3(-1.0f, 1.0f, 1.0f);
		vertex.Normal = glm::vec3(-1.0f, 0.0f, 0.0f);
		vertex.TexCoords = glm::vec2(1.0f, 0.0f);
		vertices.push_back(vertex); // top right

		vertex.Position = glm::vec3(-1.0f, 1.0f, -1.0f);
		vertex.Normal = glm::vec3(-1.0f, 0.0f, 0.0f);
		vertex.TexCoords = glm::vec2(1.0f, 1.0f);
		vertices.push_back(vertex); // top left

		vertex.Position = glm::vec3(-1.0f, -1.0f, -1.0f);
		vertex.Normal = glm::vec3(-1.0f, 0.0f, 0.0f);
		vertex.TexCoords = glm::vec2(0.0f, 1.0f);
		vertices.push_back(vertex); // bottom left

		vertex.Position = glm::vec3(-1.0f, -1.0f, 1.0f);
		vertex.Normal = glm::vec3(-1.0f, 0.0f, 0.0f);
		vertex.TexCoords = glm::vec2(0.0f, 0.0f);
		vertices.push_back(vertex); // bottom right

		// Right face
		vertex.Position = glm::vec3(1.0f, 1.0f, 1.0f);
		vertex.Normal = glm::vec3(1.0f, 0.0f, 0.0f);
		vertex.TexCoords = glm::vec2(1.0f, 0.0f);
		vertices.push_back(vertex); // top left

		vertex.Position = glm::vec3(1.0f, -1.0f, -1.0f);
		vertex.Normal = glm::vec3(1.0f, 0.0f, 0.0f);
		vertex.TexCoords = glm::vec2(0.0f, 1.0f);
		vertices.push_back(vertex); // bottom right

		vertex.Position = glm::vec3(1.0f, 1.0f, -1.0f);
		vertex.Normal = glm::vec3(1.0f, 0.0f, 0.0f);
		vertex.TexCoords = glm::vec2(1.0f, 1.0f);
		vertices.push_back(vertex); // top right

		vertex.Position = glm::vec3(1.0f, -1.0f, 1.0f);
		vertex.Normal = glm::vec3(1.0f, 0.0f, 0.0f);
		vertex.TexCoords = glm::vec2(0.0f, 0.0f);
		vertices.push_back(vertex); // bottom left

		// Bottom face
		vertex.Position = glm::vec3(-1.0f, -1.0f, -1.0f);
		vertex.Normal = glm::vec3(0.0f, -1.0f, 0.0f);
		vertex.TexCoords = glm::vec2(0.0f, 1.0f);
		vertices.push_back(vertex); // top right

		vertex.Position = glm::vec3(1.0f, -1.0f, -1.0f);
		vertex.Normal = glm::vec3(0.0f, -1.0f, 0.0f);
		vertex.TexCoords = glm::vec2(1.0f, 1.0f);
		vertices.push_back(vertex); // top left

		vertex.Position = glm::vec3(1.0f, -1.0f, 1.0f);
		vertex.Normal = glm::vec3(0.0f, -1.0f, 0.0f);
		vertex.TexCoords = glm::vec2(1.0f, 0.0f);
		vertices.push_back(vertex); // bottom left

		vertex.Position = glm::vec3(-1.0f, -1.0f, 1.0f);
		vertex.Normal = glm::vec3(0.0f, -1.0f, 0.0f);
		vertex.TexCoords = glm::vec2(0.0f, 0.0f);
		vertices.push_back(vertex); // bottom right

		// Top face
		vertex.Position = glm::vec3(-1.0f, 1.0f, -1.0f);
		vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
		vertex.TexCoords = glm::vec2(0.0f, 1.0f);
		vertices.push_back(vertex); // top left

		vertex.Position = glm::vec3(1.0f, 1.0f, 1.0f);
		vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
		vertex.TexCoords = glm::vec2(1.0f, 0.0f);
		vertices.push_back(vertex); // bottom right

		vertex.Position = glm::vec3(1.0f, 1.0f, -1.0f);
		vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
		vertex.TexCoords = glm::vec2(1.0f, 1.0f);
		vertices.push_back(vertex); // top right

		vertex.Position = glm::vec3(-1.0f, 1.0f, 1.0f);
		vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
		vertex.TexCoords = glm::vec2(0.0f, 0.0f);
		vertices.push_back(vertex); // bottom left

		indices = {
			0, 1, 2, 1, 0, 3,		// back
			4, 5, 6, 6, 7, 4,		// front
			8, 9, 10, 10, 11, 8,	// left
			12, 13, 14, 13, 12, 15, // right
			16, 17, 18, 18, 19, 16, // bottom
			20, 21, 22, 21, 20, 23  // top
		};

		GenerateBitangentTangentVectors(vertices, indices, 0);

#pragma endregion
		MeshData* defaultCubeData = new MeshData(vertices, indices);
		AddMeshData("DefaultCubeMesh", defaultCubeData, true);

		defaultCube = new Mesh(defaultCubeData);
		defaultCube->SetDrawPrimitive(GL_TRIANGLES);
		vertices.clear();
		indices.clear();
		vertex = Vertex();
#pragma region defaultPlane
		vertex.Position = glm::vec3(-1.0f, 1.0f, 0.0f);
		vertex.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
		vertex.TexCoords = glm::vec2(0.0f, 1.0f);
		vertices.push_back(vertex); // top left

		vertex.Position = glm::vec3(-1.0f, -1.0f, 0.0f);
		vertex.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
		vertex.TexCoords = glm::vec2(0.0f, 0.0f);
		vertices.push_back(vertex); // bottom left

		vertex.Position = glm::vec3(1.0f, 1.0f, 0.0f);
		vertex.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
		vertex.TexCoords = glm::vec2(1.0f, 1.0f);
		vertices.push_back(vertex); // top right

		vertex.Position = glm::vec3(1.0f, -1.0f, 0.0f);
		vertex.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
		vertex.TexCoords = glm::vec2(1.0f, 0.0f);
		vertices.push_back(vertex); // bottom right

		indices = {
			0, 1, 3,
			0, 3, 2
		};

		GenerateBitangentTangentVectors(vertices, indices, 0);
#pragma endregion
		MeshData* defaultPlaneData = new MeshData(vertices, indices);
		AddMeshData("DefaultPlaneMesh", defaultPlaneData, true);

		defaultPlane = new Mesh(defaultPlaneData);
		defaultPlane->SetDrawPrimitive(GL_TRIANGLES);
		vertices.clear();
		indices.clear();
		vertex = Vertex();
#pragma region defaultSphere
		const unsigned int X_SEGMENTS = 64;
		const unsigned int Y_SEGMENTS = 64;
		const float PI = 3.14159265359f;

		std::vector<glm::vec3> positions;
		std::vector<glm::vec2> uv;
		std::vector<glm::vec3> normals;

		for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
		{
			for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
			{
				float xSegment = (float)x / (float)X_SEGMENTS;
				float ySegment = (float)y / (float)Y_SEGMENTS;
				float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
				float yPos = std::cos(ySegment * PI);
				float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

				positions.push_back(glm::vec3(xPos, yPos, zPos));
				uv.push_back(glm::vec2(xSegment, ySegment));
				normals.push_back(glm::vec3(xPos, yPos, zPos));
			}
		}

		bool oddRow = false;
		for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
		{
			if (!oddRow)
			{
				for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
				{
					indices.push_back(y * (X_SEGMENTS + 1) + x);
					indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
				}
			}
			else
			{
				for (int x = X_SEGMENTS; x >= 0; --x)
				{
					indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
					indices.push_back(y * (X_SEGMENTS + 1) + x);
				}
			}
			oddRow = !oddRow;
		}

		for (unsigned int i = 0; i < positions.size(); i++) {
			vertex = Vertex();
			vertex.Position = positions[i];
			vertex.Normal = normals[i];
			vertex.TexCoords = uv[i];
			vertices.push_back(vertex);
		}

		GenerateBitangentTangentVectors(vertices, indices, 1);
#pragma endregion
		MeshData* defaultSphereData = new MeshData(vertices, indices);
		AddMeshData("DefaultSphereMesh", defaultSphereData, true);

		defaultSphere = new Mesh(vertices, indices);
		defaultSphere->SetDrawPrimitive(GL_TRIANGLE_STRIP);

		vertices.clear();
		indices.clear();
		vertex = Vertex();

		// Setup default ui quad mesh
		// --------------------------
		glm::vec4 uiQuadVertices[4] = {
			glm::vec4(-1.0f, 1.0f, 0.0f, 1.0f), // top-left
			glm::vec4(-1.0f, -1.0f, 0.0f, 0.0f), // bottom-left
			glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), // top-right
			glm::vec4(1.0f, -1.0f, 1.0f, 0.0f) // bottom-right
		};
		unsigned int uiQuadIndices[6] = {	
			0, 1, 3,
			3, 1, 2
		};

		glGenVertexArrays(1, &uiQuadVAO);
		glGenBuffers(1, &uiQuadVBO);

		glBindVertexArray(uiQuadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, uiQuadVBO);

		glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec4), &uiQuadVertices[0], GL_STATIC_DRAW);

		// vertex positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);

		// texture coords
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)(sizeof(glm::vec2)));

		// Set up single point VBO
		glGenVertexArrays(1, &pointVAO);
		glGenBuffers(1, &pointVBO);
		glBindVertexArray(pointVAO);
		glBindBuffer(GL_ARRAY_BUFFER, pointVBO);

		glm::vec3 point = glm::vec3(0.0f);

		glBufferData(GL_ARRAY_BUFFER, sizeof(point), &point, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		int textureOffset = 10;

		shadowMapShader = LoadShader("Shaders/depthMap.vert", "Shaders/depthMap.frag", true);
		cubeShadowMapShader = LoadShader("Shaders/cubeDepthMap.vert", "Shaders/cubeDepthMap.frag", "Shaders/cubeDepthMap.geom", true);
		defaultLitShader = LoadShader("Shaders/defaultLitNew.vert", "Shaders/defaultLitNew.frag", true);
		screenQuadShader = LoadShader("Shaders/screenQuad.vert", "Shaders/screenQuad.frag", true);
		hdrTonemappingShader = LoadShader("Shaders/screenQuad.vert", "Shaders/hdrTonemapping.frag", true);
		deferredGeometryPass = LoadShader("Shaders/g_buffer.vert", "Shaders/g_buffer.frag", true);
		deferredGeometryPassPBR = LoadShader("Shaders/g_buffer.vert", "Shaders/g_bufferPBR.frag", true);
		deferredLightingPass = LoadShader("Shaders/defaultDeferred.vert", "Shaders/defaultDeferred.frag", true);
		deferredLightingPassPBR = LoadShader("Shaders/defaultDeferred.vert", "Shaders/defaultDeferredPBR.frag", true);
		ssaoShader = LoadShader("Shaders/ssao.vert", "Shaders/ssao.frag", true);
		ssaoBlur = LoadShader("Shaders/ssao.vert", "Shaders/ssaoBlur.frag", true);
		skyboxShader = LoadShader("Shaders/skybox.vert", "Shaders/skybox.frag", true);
		defaultLitPBRShader = LoadShader("Shaders/defaultLit_pbr.vert", "Shaders/defaultLit_pbr.frag", true);
		bloomBlur = LoadShader("Shaders/bloomBlur.vert", "Shaders/bloomBlur.frag", true);
		equirectangularToCubemapShader = LoadShader("Shaders/convertToCubemap.vert", "Shaders/convertToCubemap.frag", true);
		createIrradianceShader = LoadShader("Shaders/convertToCubemap.vert", "Shaders/irradianceConvolute.frag", true);
		createPrefilterShader = LoadShader("Shaders/convertToCubemap.vert", "Shaders/prefilter.frag", true);
		createBRDFShader = LoadShader("Shaders/brdf.vert", "Shaders/brdf.frag", true);
		defaultTextShader = LoadShader("Shaders/defaultText.vert", "Shaders/defaultText.frag", true);
		defaultImageShader = LoadShader("Shaders/defaultImage.vert", "Shaders/defaultImage.frag", true);
		uiBackgroundShader = LoadShader("Shaders/uiBackground.vert", "Shaders/uiBackground.frag", "Shaders/uiBackground.geom", true);
		particleShader = LoadShader("Shaders/particles.vert", "Shaders/particles.frag", true);
		pointParticleShader = LoadShader("Shaders/pointParticles.vert", "Shaders/particles.frag", "Shaders/pointParticles.geom", true);
		advBloomDownsampleShader = LoadShader("Shaders/screenQuad.vert", "Shaders/advBloomDownsample.frag", true);
		advBloomUpsampleShader = LoadShader("Shaders/screenQuad.vert", "Shaders/advBloomUpsample.frag", true);
		advBloomCombineShader = LoadShader("Shaders/screenQuad.vert", "Shaders/advBloomCombine.frag", true);
		reflectionProbeBaking = LoadShader("Shaders/reflectionProbeBakingPBR.vert", "Shaders/reflectionProbeBaking.frag", true);
		reflectionProbeBakingPBR = LoadShader("Shaders/reflectionProbeBakingPBR.vert", "Shaders/reflectionProbeBakingPBR.frag", true);
		colliderDebug = LoadShader("Shaders/colliderDebug.vert", "Shaders/colliderDebug.frag", "Shaders/colliderDebug.geom", true);
		screenSpaceReflectionUVMappingShader = LoadShader("Shaders/ssao.vert", "Shaders/screenSpaceReflectionUVMapping.frag", true);
		ssrUVMapToReflectionMap = LoadShader("Shaders/screenQuad.vert", "Shaders/ssrUVToReflectionMap.frag", true);

		ssrUVMapToReflectionMap->Use();
		ssrUVMapToReflectionMap->setInt("colourMap", 0);
		ssrUVMapToReflectionMap->setInt("uvMap", 1);

		screenSpaceReflectionUVMappingShader->Use();
		screenSpaceReflectionUVMappingShader->setInt("gPosition", 0);
		screenSpaceReflectionUVMappingShader->setInt("gNormal", 1);

		shadowMapShader->Use();
		shadowMapShader->setInt("material.TEXTURE_OPACITY1", textureSlotLookup.at("material.TEXTURE_OPACITY1"));

		cubeShadowMapShader->Use();
		cubeShadowMapShader->setInt("material.TEXTURE_OPACITY1", textureSlotLookup.at("material.TEXTURE_OPACITY1"));

		advBloomCombineShader->Use();
		advBloomCombineShader->setInt("screenTexture", 0);
		advBloomCombineShader->setInt("bloomTexture", 1);
		advBloomCombineShader->setInt("dirtMask", 2);
		
		advBloomDownsampleShader->Use();
		advBloomDownsampleShader->setInt("srcTexture", 0);

		advBloomUpsampleShader->Use();
		advBloomUpsampleShader->setInt("srcTexture", 0);

		bloomBlur->Use();
		bloomBlur->setInt("image", 0);

		screenQuadShader->Use();
		screenQuadShader->setInt("screenTexture", 0);

		hdrTonemappingShader->Use();
		hdrTonemappingShader->setInt("screenTexture", 0);
		hdrTonemappingShader->setInt("bloomTexture", 1);

		// Default lit
		// -----------
		defaultLitShader->Use();
		defaultLitShader->setInt("dirLight.ShadowMap", textureSlotLookup.at("dirLight.ShadowMap"));
		defaultLitShader->setInt("spotlightShadowAtlas", textureSlotLookup.at("spotlightShadowAtlas"));
		defaultLitShader->setInt("pointLightShadowArray", textureSlotLookup.at("pointLightShadowArray"));

		defaultLitShader->setInt("material.TEXTURE_DIFFUSE1", textureSlotLookup.at("material.TEXTURE_DIFFUSE1"));
		defaultLitShader->setInt("material.TEXTURE_SPECULAR1", textureSlotLookup.at("material.TEXTURE_SPECULAR1"));
		defaultLitShader->setInt("material.TEXTURE_NORMAL1", textureSlotLookup.at("material.TEXTURE_NORMAL1"));
		defaultLitShader->setInt("material.TEXTURE_DISPLACE1", textureSlotLookup.at("material.TEXTURE_DISPLACE1"));
		defaultLitShader->setInt("material.TEXTURE_OPACITY1", textureSlotLookup.at("material.TEXTURE_OPACITY1"));

		deferredGeometryPass->Use();
		deferredGeometryPass->setInt("material.TEXTURE_DIFFUSE1", textureSlotLookup.at("material.TEXTURE_DIFFUSE1"));
		deferredGeometryPass->setInt("material.TEXTURE_SPECULAR1", textureSlotLookup.at("material.TEXTURE_SPECULAR1"));
		deferredGeometryPass->setInt("material.TEXTURE_NORMAL1", textureSlotLookup.at("material.TEXTURE_NORMAL1"));
		deferredGeometryPass->setInt("material.TEXTURE_DISPLACE1", textureSlotLookup.at("material.TEXTURE_DISPLACE1"));
		deferredGeometryPass->setInt("material.TEXTURE_OPACITY1", textureSlotLookup.at("material.TEXTURE_OPACITY1"));

		deferredGeometryPassPBR->Use();
		deferredGeometryPassPBR->setInt("material.TEXTURE_ALBEDO1", textureSlotLookup.at("material.TEXTURE_ALBEDO1"));
		deferredGeometryPassPBR->setInt("material.TEXTURE_NORMAL1", textureSlotLookup.at("material.TEXTURE_NORMAL1"));
		deferredGeometryPassPBR->setInt("material.TEXTURE_METALLIC1", textureSlotLookup.at("material.TEXTURE_METALLIC1"));
		deferredGeometryPassPBR->setInt("material.TEXTURE_ROUGHNESS1", textureSlotLookup.at("material.TEXTURE_ROUGHNESS1"));
		deferredGeometryPassPBR->setInt("material.TEXTURE_AO1", textureSlotLookup.at("material.TEXTURE_AO1"));
		deferredGeometryPassPBR->setInt("material.TEXTURE_DISPLACE1", textureSlotLookup.at("material.TEXTURE_DISPLACE1"));
		deferredGeometryPassPBR->setInt("material.TEXTURE_OPACITY1", textureSlotLookup.at("material.TEXTURE_OPACITY1"));

		deferredLightingPass->Use();
		deferredLightingPass->setInt("dirLight.ShadowMap", textureSlotLookup.at("dirLight.ShadowMap"));
		deferredLightingPass->setInt("spotlightShadowAtlas", textureSlotLookup.at("spotlightShadowAtlas"));
		deferredLightingPass->setInt("pointLightShadowArray", textureSlotLookup.at("pointLightShadowArray"));

		deferredLightingPass->setInt("gPosition", textureSlotLookup.at("gPosition"));
		deferredLightingPass->setInt("gNormal", textureSlotLookup.at("gNormal"));
		deferredLightingPass->setInt("gAlbedo", textureSlotLookup.at("gAlbedo"));
		deferredLightingPass->setInt("gSpecular", textureSlotLookup.at("gSpecular"));
		deferredLightingPass->setInt("gPBRFLAG", textureSlotLookup.at("gPBRFLAG"));
		deferredLightingPass->setInt("SSAO", textureSlotLookup.at("SSAO"));

		deferredLightingPassPBR->Use();
		deferredLightingPassPBR->setInt("dirLight.ShadowMap", textureSlotLookup.at("dirLight.ShadowMap"));
		deferredLightingPassPBR->setInt("spotlightShadowAtlas", textureSlotLookup.at("spotlightShadowAtlas"));
		deferredLightingPassPBR->setInt("pointLightShadowArray", textureSlotLookup.at("pointLightShadowArray"));

		deferredLightingPassPBR->setInt("gPosition", textureSlotLookup.at("gPosition"));
		deferredLightingPassPBR->setInt("gNormal", textureSlotLookup.at("gNormal"));
		deferredLightingPassPBR->setInt("gAlbedo", textureSlotLookup.at("gAlbedo"));
		deferredLightingPassPBR->setInt("gArm", textureSlotLookup.at("gArm"));
		deferredLightingPassPBR->setInt("gPBRFLAG", textureSlotLookup.at("gPBRFLAG"));
		deferredLightingPassPBR->setInt("SSAO", textureSlotLookup.at("SSAO"));

		deferredLightingPassPBR->setInt("globalIBL.irradianceMap", textureSlotLookup.at("globalIBL.irradianceMap"));
		deferredLightingPassPBR->setInt("globalIBL.prefilterMap", textureSlotLookup.at("globalIBL.prefilterMap"));
		deferredLightingPassPBR->setInt("brdfLUT", textureSlotLookup.at("brdfLUT"));

		deferredLightingPassPBR->setInt("localIBLIrradianceMapArray", textureSlotLookup.at("localIBLIrradianceMapArray"));
		deferredLightingPassPBR->setInt("localIBLPrefilterMapArray", textureSlotLookup.at("localIBLPrefilterMapArray"));

		deferredLightingPassPBR->setInt("nonPBRResult", 30);
		deferredLightingPassPBR->setInt("nonPBRBrightResult", 31);

		ssaoShader->Use();
		ssaoShader->setInt("gPosition", 0);
		ssaoShader->setInt("gNormal", 1);
		ssaoShader->setInt("texNoise", 2);
		
		ssaoBlur->Use();
		ssaoBlur->setInt("ssaoInput", 0);

		skyboxShader->Use();
		skyboxShader->setInt("cubemap", 0);

		defaultLitPBRShader->Use();

		defaultLitPBRShader->setInt("dirLight.ShadowMap", textureSlotLookup.at("dirLight.ShadowMap"));
		defaultLitPBRShader->setInt("spotlightShadowAtlas", textureSlotLookup.at("spotlightShadowAtlas"));
		defaultLitPBRShader->setInt("pointLightShadowArray", textureSlotLookup.at("pointLightShadowArray"));

		defaultLitPBRShader->setInt("material.TEXTURE_ALBEDO1", textureSlotLookup.at("material.TEXTURE_ALBEDO1"));
		defaultLitPBRShader->setInt("material.TEXTURE_NORMAL1", textureSlotLookup.at("material.TEXTURE_NORMAL1"));
		defaultLitPBRShader->setInt("material.TEXTURE_METALLIC1", textureSlotLookup.at("material.TEXTURE_METALLIC1"));
		defaultLitPBRShader->setInt("material.TEXTURE_ROUGHNESS1", textureSlotLookup.at("material.TEXTURE_ROUGHNESS1"));
		defaultLitPBRShader->setInt("material.TEXTURE_AO1", textureSlotLookup.at("material.TEXTURE_AO1"));
		defaultLitPBRShader->setInt("material.TEXTURE_DISPLACE1", textureSlotLookup.at("material.TEXTURE_DISPLACE1"));
		defaultLitPBRShader->setInt("material.TEXTURE_OPACITY1", textureSlotLookup.at("material.TEXTURE_OPACITY1"));

		defaultLitPBRShader->setInt("globalIBL.irradianceMap", textureSlotLookup.at("globalIBL.irradianceMap"));
		defaultLitPBRShader->setInt("globalIBL.prefilterMap", textureSlotLookup.at("globalIBL.prefilterMap"));
		defaultLitPBRShader->setInt("brdfLUT", textureSlotLookup.at("brdfLUT"));

		defaultLitPBRShader->setInt("localIBLIrradianceMapArray", textureSlotLookup.at("localIBLIrradianceMapArray"));
		defaultLitPBRShader->setInt("localIBLPrefilterMapArray", textureSlotLookup.at("localIBLPrefilterMapArray"));

		reflectionProbeBakingPBR->Use();
		reflectionProbeBakingPBR->setInt("dirLight.ShadowMap", textureSlotLookup.at("dirLight.ShadowMap"));
		reflectionProbeBakingPBR->setInt("spotlightShadowAtlas", textureSlotLookup.at("spotlightShadowAtlas"));
		reflectionProbeBakingPBR->setInt("pointLightShadowArray", textureSlotLookup.at("pointLightShadowArray"));

		reflectionProbeBakingPBR->setInt("material.TEXTURE_ALBEDO1", textureSlotLookup.at("material.TEXTURE_ALBEDO1"));
		reflectionProbeBakingPBR->setInt("material.TEXTURE_NORMAL1", textureSlotLookup.at("material.TEXTURE_NORMAL1"));
		reflectionProbeBakingPBR->setInt("material.TEXTURE_METALLIC1", textureSlotLookup.at("material.TEXTURE_METALLIC1"));
		reflectionProbeBakingPBR->setInt("material.TEXTURE_ROUGHNESS1", textureSlotLookup.at("material.TEXTURE_ROUGHNESS1"));
		reflectionProbeBakingPBR->setInt("material.TEXTURE_AO1", textureSlotLookup.at("material.TEXTURE_AO1"));
		reflectionProbeBakingPBR->setInt("material.TEXTURE_DISPLACE1", textureSlotLookup.at("material.TEXTURE_DISPLACE1"));
		reflectionProbeBakingPBR->setInt("material.TEXTURE_OPACITY1", textureSlotLookup.at("material.TEXTURE_OPACITY1"));

		reflectionProbeBaking->Use();
		reflectionProbeBaking->setInt("dirLight.ShadowMap", textureSlotLookup.at("dirLight.ShadowMap"));
		reflectionProbeBaking->setInt("spotlightShadowAtlas", textureSlotLookup.at("spotlightShadowAtlas"));
		reflectionProbeBaking->setInt("pointLightShadowArray", textureSlotLookup.at("pointLightShadowArray"));

		reflectionProbeBaking->setInt("material.TEXTURE_DIFFUSE1", textureSlotLookup.at("material.TEXTURE_DIFFUSE1"));
		reflectionProbeBaking->setInt("material.TEXTURE_SPECULAR1", textureSlotLookup.at("material.TEXTURE_SPECULAR1"));
		reflectionProbeBaking->setInt("material.TEXTURE_NORMAL1", textureSlotLookup.at("material.TEXTURE_NORMAL1"));
		reflectionProbeBaking->setInt("material.TEXTURE_DISPLACE1", textureSlotLookup.at("material.TEXTURE_DISPLACE1"));
		reflectionProbeBaking->setInt("material.TEXTURE_OPACITY1", textureSlotLookup.at("material.TEXTURE_OPACITY1"));

		defaultTextShader->Use();
		defaultTextShader->setInt("text", 0);

		defaultImageShader->Use();
		defaultImageShader->setInt("image", 0);

		particleShader->Use();
		particleShader->setInt("sprite", 0);

		pointParticleShader->Use();
		pointParticleShader->setInt("sprite", 0);

		// Uniform blocks
		unsigned int defaultLitBlockLocation = glGetUniformBlockIndex(defaultLitShader->GetID(), "Common");
		unsigned int deferredGeometryPassLocation = glGetUniformBlockIndex(deferredGeometryPass->GetID(), "Common");
		unsigned int deferredGeometryPassPBRLocation = glGetUniformBlockIndex(deferredGeometryPassPBR->GetID(), "Common");
		unsigned int deferredLightingPassLocation = glGetUniformBlockIndex(deferredLightingPass->GetID(), "Common");
		unsigned int deferredLightingPassPBRLocation = glGetUniformBlockIndex(deferredLightingPassPBR->GetID(), "Common");
		unsigned int ssaoShaderLocation = glGetUniformBlockIndex(ssaoShader->GetID(), "Common");
		unsigned int skyboxShaderLocation = glGetUniformBlockIndex(skyboxShader->GetID(), "Common");
		unsigned int defaultLitPBRBlockLocation = glGetUniformBlockIndex(defaultLitPBRShader->GetID(), "Common");
		unsigned int defaultParticleLocation = glGetUniformBlockIndex(particleShader->GetID(), "Common");
		unsigned int defaultPointParticleLocation = glGetUniformBlockIndex(pointParticleShader->GetID(), "Common");
		unsigned int colliderDebugLocation = glGetUniformBlockIndex(colliderDebug->GetID(), "Common");
		unsigned int screenSpaceUVLocation = glGetUniformBlockIndex(screenSpaceReflectionUVMappingShader->GetID(), "Common");
		glUniformBlockBinding(defaultLitShader->GetID(), defaultLitBlockLocation, 0);
		glUniformBlockBinding(deferredGeometryPass->GetID(), deferredGeometryPassLocation, 0);
		glUniformBlockBinding(deferredGeometryPassPBR->GetID(), deferredGeometryPassPBRLocation, 0);
		glUniformBlockBinding(deferredLightingPass->GetID(), deferredLightingPassLocation, 0);
		glUniformBlockBinding(deferredLightingPassPBR->GetID(), deferredLightingPassPBRLocation, 0);
		glUniformBlockBinding(ssaoShader->GetID(), ssaoShaderLocation, 0);
		glUniformBlockBinding(skyboxShader->GetID(), skyboxShaderLocation, 0);
		glUniformBlockBinding(defaultLitPBRShader->GetID(), defaultLitPBRBlockLocation, 0);
		glUniformBlockBinding(particleShader->GetID(), defaultParticleLocation, 0);
		glUniformBlockBinding(pointParticleShader->GetID(), defaultPointParticleLocation, 0);
		glUniformBlockBinding(colliderDebug->GetID(), colliderDebugLocation, 0);
		glUniformBlockBinding(screenSpaceReflectionUVMappingShader->GetID(), screenSpaceUVLocation, 0);

		glGenBuffers(1, &uboMatrices);
		glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
		glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4) + sizeof(glm::vec3), NULL, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4) + sizeof(glm::vec3));
	}

	ResourceManager::~ResourceManager()
	{
		// delete defaults
		delete defaultCube;
		delete defaultPlane;
		delete defaultSphere;
		delete defaultMaterial;

		ClearTempResources();
		ClearPersistentResources();

		FT_Done_FreeType(freetypeLib);

		delete instance;
	}

	ResourceManager* ResourceManager::GetInstance()
	{
		if (instance == nullptr) {
			instance = new ResourceManager();
		}
		return instance;
	}

	void ResourceManager::GenerateBitangentTangentVectors(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices, unsigned int offset)
	{
		// Calculate tangent and bitangent vectors
		for (unsigned int i = 0; i < indices.size() - offset; i += 3) {
			Vertex& v0 = vertices[indices[i]];
			Vertex& v1 = vertices[indices[i + 1]];
			Vertex& v2 = vertices[indices[i + 2]];

			// Edges of triangle
			glm::vec3 deltaPos1 = v1.Position - v0.Position;
			glm::vec3 deltaPos2 = v2.Position - v0.Position;

			// UV delta
			glm::vec2 deltaUV1 = v1.TexCoords - v0.TexCoords;
			glm::vec2 deltaUV2 = v2.TexCoords - v0.TexCoords;

			// Tangent and bitangent
			float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
			glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
			glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;

			v0.Tangent += tangent;
			v1.Tangent += tangent;
			v2.Tangent += tangent;
			v0.Bitangent += bitangent;
			v1.Bitangent += bitangent;
			v2.Bitangent += bitangent;
		}

		// Normalize tangent and bitangent vectors
		for (unsigned int i = 0; i < vertices.size(); ++i) {
			vertices[i].Tangent = glm::normalize(vertices[i].Tangent);
			vertices[i].Bitangent = glm::normalize(vertices[i].Bitangent);
		}
	}

	void ResourceManager::ClearResources(Resources& resources)
	{
		// delete meshes
		std::unordered_map<std::string, MeshData*>::iterator meshesIt = resources.meshes.begin();
		while (meshesIt != resources.meshes.end()) {
			delete meshesIt->second;
			meshesIt++;
		}
		resources.meshes.clear();

		// delete shaders
		std::unordered_map<std::string, AbstractShader*>::iterator shadersIt = resources.shaders.begin();
		while (shadersIt != resources.shaders.end()) {
			delete shadersIt->second;
			shadersIt++;
		}
		resources.shaders.clear();

		// delete textures
		std::unordered_map<std::string, Texture*>::iterator texturesIt = resources.textures.begin();
		while (texturesIt != resources.textures.end()) {
			delete texturesIt->second;
			texturesIt++;
		}
		resources.textures.clear();

		// delete cubemaps
		std::unordered_map<std::string, Cubemap*>::iterator cubemapsIt = resources.cubemaps.begin();
		while (cubemapsIt != resources.cubemaps.end()) {
			delete cubemapsIt->second;
			cubemapsIt++;
		}
		resources.cubemaps.clear();

		// delete hdr cubemaps
		std::unordered_map<std::string, HDREnvironment*>::iterator hdrmapsIt = resources.hdrCubemaps.begin();
		while (hdrmapsIt != resources.hdrCubemaps.end()) {
			delete hdrmapsIt->second;
			hdrmapsIt++;
		}
		resources.hdrCubemaps.clear();

		// delete fonts
		std::unordered_map<std::string, TextFont*>::iterator textFontsIt = resources.textFonts.begin();
		while (textFontsIt != resources.textFonts.end()) {
			delete textFontsIt->second;
			textFontsIt++;
		}
		resources.textFonts.clear();

		// delete animations
		std::unordered_map<std::string, SkeletalAnimation*>::iterator animationsIt = resources.animations.begin();
		while (animationsIt != resources.animations.end()) {
			delete animationsIt->second;
			animationsIt++;
		}
		resources.animations.clear();

		// delete audio files
		std::unordered_map<std::string, AudioFile*>::iterator audioIt = resources.audioFiles.begin();
		while (audioIt != resources.audioFiles.end()) {
			delete audioIt->second;
			audioIt++;
		}
		resources.audioFiles.clear();

		// delete materials
		std::unordered_map<std::string, AbstractMaterial*>::iterator materialsIt = resources.materials.begin();
		while (materialsIt != resources.materials.end()) {
			delete materialsIt->second;
			materialsIt++;
		}
		resources.materials.clear();

		// delete skeletons
		std::unordered_map<std::string, AnimationSkeleton*>::iterator skeletonsIt = resources.animationSkeletons.begin();
		while (skeletonsIt != resources.animationSkeletons.end()) {
			delete skeletonsIt->second;
			skeletonsIt++;
		}
		resources.animationSkeletons.clear();
	}


	Model* ResourceManager::CreateModel(const std::string& filepath, bool pbr, bool loadInPersistentResources, const unsigned int assimpPostProcess)
	{
		ASSIMPModelLoader::loadMaterialsAsPBR = pbr;
		LoadMeshDataResult meshData = ASSIMPModelLoader::LoadMeshData(filepath, assimpPostProcess, loadInPersistentResources);
		std::vector<Mesh*> meshes;
		meshes.reserve(meshData.meshResults.size());

		for (unsigned int i = 0; i < meshData.meshResults.size(); i++) {
			meshes.push_back(new Mesh(meshData.meshResults[i].meshData, meshData.meshResults[i].meshMaterials));
		}

		Model* newModel = new Model(meshes, pbr);
		newModel->SetHasBones(meshData.hasBones);
		newModel->SetAnimationSkeleton(meshData.skeleton);

		return newModel;
	}

	ComputeShader* ResourceManager::LoadComputeShader(const char* computePath, bool loadInPersistentResources)
	{
		std::unordered_map<std::string, AbstractShader*>::iterator persistentIt = persistentResources.shaders.find(computePath);
		std::unordered_map<std::string, AbstractShader*>::iterator tempIt = tempResources.shaders.find(computePath);

		bool existsInPersistent = true;
		bool existsInTemp = true;
		if (persistentIt == persistentResources.shaders.end()) {
			existsInPersistent = false;
		}
		if (tempIt == tempResources.shaders.end()) {
			existsInTemp = false;
		}

		if (!existsInPersistent && !existsInTemp) {
			std::cout << "RESOURCEMANAGER::Loading compute shader " << computePath << std::endl;
			// Shader not currently loaded
			if (loadInPersistentResources) {
				persistentResources.shaders[computePath] = dynamic_cast<AbstractShader*>(new ComputeShader(computePath));
				return dynamic_cast<ComputeShader*>(persistentResources.shaders[computePath]);
			}
			else {
				tempResources.shaders[computePath] = dynamic_cast<AbstractShader*>(new ComputeShader(computePath));
				return dynamic_cast<ComputeShader*>(tempResources.shaders[computePath]);
			}
		}

		std::unordered_map<std::string, AbstractShader*>::iterator it = persistentIt;
		if (existsInTemp) { it = tempIt; }
		return dynamic_cast<ComputeShader*>(it->second);
	}

	Shader* ResourceManager::LoadShader(std::string vertexPath, std::string fragmentPath, bool loadInPersistentResources)
	{
		std::string combinedPath = vertexPath + "|" + fragmentPath;

		std::unordered_map<std::string, AbstractShader*>::iterator persistentIt = persistentResources.shaders.find(combinedPath);
		std::unordered_map<std::string, AbstractShader*>::iterator tempIt = tempResources.shaders.find(combinedPath);

		bool existsInPersistent = true;
		bool existsInTemp = true;
		if (persistentIt == persistentResources.shaders.end()) {
			existsInPersistent = false;
		}
		if (tempIt == tempResources.shaders.end()) {
			existsInTemp = false;
		}

		if (!existsInPersistent && !existsInTemp) {
			std::cout << "RESOURCEMANAGER::Loading shader " << combinedPath << std::endl;
			// Shader not currently loaded
			if (loadInPersistentResources) {
				persistentResources.shaders[combinedPath] = dynamic_cast<AbstractShader*>(new Shader(vertexPath.c_str(), fragmentPath.c_str()));
				return dynamic_cast<Shader*>(persistentResources.shaders[combinedPath]);
			}
			else {
				tempResources.shaders[combinedPath] = new Shader(vertexPath.c_str(), fragmentPath.c_str());
				return dynamic_cast<Shader*>(tempResources.shaders[combinedPath]);
			}
		}

		std::unordered_map<std::string, AbstractShader*>::iterator it = persistentIt;
		if (existsInTemp) { it = tempIt; }
		return dynamic_cast<Shader*>(it->second);
	}

	Shader* ResourceManager::LoadShader(std::string vertexPath, std::string fragmentPath, std::string geometryPath, bool loadInPersistentResources)
	{
		std::string combinedPath = vertexPath + "|" + fragmentPath + "|" + geometryPath;

		std::unordered_map<std::string, AbstractShader*>::iterator persistentIt = persistentResources.shaders.find(combinedPath);
		std::unordered_map<std::string, AbstractShader*>::iterator tempIt = tempResources.shaders.find(combinedPath);

		bool existsInPersistent = true;
		bool existsInTemp = true;
		if (persistentIt == persistentResources.shaders.end()) {
			existsInPersistent = false;
		}
		if (tempIt == tempResources.shaders.end()) {
			existsInTemp = false;
		}

		if (!existsInPersistent && !existsInTemp) {
			std::cout << "RESOURCEMANAGER::Loading shader " << combinedPath << std::endl;
			// Shader not currently loaded
			if (loadInPersistentResources) {
				persistentResources.shaders[combinedPath] = dynamic_cast<AbstractShader*>(new Shader(vertexPath.c_str(), fragmentPath.c_str(), geometryPath.c_str()));
				return dynamic_cast<Shader*>(persistentResources.shaders[combinedPath]);
			}
			else {
				tempResources.shaders[combinedPath] = dynamic_cast<AbstractShader*>(new Shader(vertexPath.c_str(), fragmentPath.c_str(), geometryPath.c_str()));
				return dynamic_cast<Shader*>(tempResources.shaders[combinedPath]);
			}
		}

		std::unordered_map<std::string, AbstractShader*>::iterator it = persistentIt;
		if (existsInTemp) { it = tempIt; }
		return dynamic_cast<Shader*>(it->second);
	}

	Texture* ResourceManager::LoadTexture(std::string filepath, TextureTypes type, bool srgb, bool loadInPersistentResources, AnisotropicFiltering anisoFilter)
	{
		// First check if already loaded
		std::unordered_map<std::string, Texture*>::iterator persistentIt = persistentResources.textures.find(filepath);
		std::unordered_map<std::string, Texture*>::iterator tempIt = tempResources.textures.find(filepath);

		bool existsInPersistent = true;
		bool existsInTemp = true;
		if (persistentIt == persistentResources.textures.end()) {
			existsInPersistent = false;
		}
		if (tempIt == tempResources.textures.end()) {
			existsInTemp = false;
		}

		if (!existsInPersistent && !existsInTemp) {
			std::cout << "RESOURCEMANAGER::Loading texture " << filepath << std::endl;
			// Load texture
			unsigned int textureID;
			glGenTextures(1, &textureID);

			int width, height, nrComponents;
			unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &nrComponents, 0);
			if (data) {
				GLenum internalFormat = GL_RGB;
				GLenum dataFormat = GL_RGB;
				if (nrComponents == 1) {
					internalFormat = GL_RED;
					dataFormat = GL_RED;
				}
				else if (nrComponents == 2) {
					internalFormat = GL_RG;
					dataFormat = GL_RG;
				}
				else if (nrComponents == 3) {
					internalFormat = srgb ? GL_SRGB : GL_RGB;
					dataFormat = GL_RGB;
				}
				else if (nrComponents == 4) {
					internalFormat = srgb ? GL_SRGB_ALPHA : GL_RGBA;
					dataFormat = GL_RGBA;
				}
				glBindTexture(GL_TEXTURE_2D, textureID);
				glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
				glGenerateMipmap(GL_TEXTURE_2D);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				if (anisoFilter == ANISO_DEFAULT) { anisoFilter = RenderManager::GetInstance()->GetRenderParams()->GetDefaultAnisoFiltering(); }
				float maxAnisoFilter;
				glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAnisoFilter);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, std::min(maxAnisoFilter, (float)anisoFilter));

				stbi_image_free(data);
			}
			else {
				std::cout << "ERROR::RESOURCEMANAGER::TEXTURELOAD::Texture failed to load at path: " << filepath << std::endl;
				stbi_image_free(data);
			}

			Texture* texture = new Texture();
			texture->id = textureID;
			texture->type = type;
			texture->filepath = filepath;

			if (loadInPersistentResources) {
				persistentResources.textures[filepath] = texture;
				return persistentResources.textures[filepath];
			}
			else {
				tempResources.textures[filepath] = texture;
				return tempResources.textures[filepath];
			}
		}

		std::unordered_map<std::string, Texture*>::iterator it = persistentIt;
		if (existsInTemp) { it = tempIt; }
		return it->second;
	}

	Texture* ResourceManager::LoadTextureEmbedded(const aiTexture* embeddedTexture, const std::string& filename, TextureTypes type, bool srgb, bool loadInPersistentResources)
	{
		// First check if already loaded
		std::unordered_map<std::string, Texture*>::iterator persistentIt = persistentResources.textures.find(filename);
		std::unordered_map<std::string, Texture*>::iterator tempIt = tempResources.textures.find(filename);

		bool existsInPersistent = true;
		bool existsInTemp = true;
		if (persistentIt == persistentResources.textures.end()) {
			existsInPersistent = false;
		}
		if (tempIt == tempResources.textures.end()) {
			existsInTemp = false;
		}

		if (!existsInPersistent && !existsInTemp) {
			std::cout << "RESOURCEMANAGER::Loading embedded texture " << filename << std::endl;
		
			// Load texture
			unsigned int textureID;
			glGenTextures(1, &textureID);

			int width, height, nrComponents;
			unsigned char* data = nullptr;
			if (embeddedTexture->mHeight == 0) {
				data = stbi_load_from_memory(reinterpret_cast<unsigned char*>(embeddedTexture->pcData), embeddedTexture->mWidth, &width, &height, &nrComponents, 0);
			}
			else {
				data = stbi_load_from_memory(reinterpret_cast<unsigned char*>(embeddedTexture->pcData), embeddedTexture->mWidth * embeddedTexture->mHeight, &width, &height, &nrComponents, 0);
			}

			if (data) {
				GLenum internalFormat = GL_RGB;
				GLenum dataFormat = GL_RGB;
				if (nrComponents == 1) {
					internalFormat = GL_RED;
					dataFormat = GL_RED;
				}
				else if (nrComponents == 2) {
					internalFormat = GL_RG;
					dataFormat = GL_RG;
				}
				else if (nrComponents == 3) {
					internalFormat = srgb ? GL_SRGB : GL_RGB;
					dataFormat = GL_RGB;
				}
				else if (nrComponents == 4) {
					internalFormat = srgb ? GL_SRGB_ALPHA : GL_RGBA;
					dataFormat = GL_RGBA;
				}
				glBindTexture(GL_TEXTURE_2D, textureID);
				glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
				glGenerateMipmap(GL_TEXTURE_2D);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				float maxAnisoFilter;
				glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAnisoFilter);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, std::min(maxAnisoFilter, (float)RenderManager::GetInstance()->GetRenderParams()->GetDefaultAnisoFiltering()));

				stbi_image_free(data);
			}
			else {
				std::cout << "ERROR::RESOURCEMANAGER::EMBEDDEDTEXTURELOAD::Texture: " << filename << " failed to load" << std::endl;
				stbi_image_free(data);
			}

			Texture* texture = new Texture();
			texture->id = textureID;
			texture->type = type;
			texture->filepath = filename;

			if (loadInPersistentResources) {
				persistentResources.textures[filename] = texture;
				return persistentResources.textures[filename];
			}
			else {
				tempResources.textures[filename] = texture;
				return tempResources.textures[filename];
			}
		}

		std::unordered_map<std::string, Texture*>::iterator it = persistentIt;
		if (existsInTemp) { it = tempIt; }
		return it->second;
	}

	Cubemap* ResourceManager::LoadCubemap(const std::string& rootFilepath, const std::string& extension, bool loadInPersistentResources)
	{
		// First check if already loaded
		std::unordered_map<std::string, Cubemap*>::iterator persistentIt = persistentResources.cubemaps.find(rootFilepath);
		std::unordered_map<std::string, Cubemap*>::iterator tempIt = tempResources.cubemaps.find(rootFilepath);

		bool existsInPersistent = true;
		bool existsInTemp = true;
		if (persistentIt == persistentResources.cubemaps.end()) {
			existsInPersistent = false;
		}
		if (tempIt == tempResources.cubemaps.end()) {
			existsInTemp = false;
		}

		if (!existsInPersistent && !existsInTemp) {
			std::cout << "RESOURCEMANAGER::Loading cubemap root " << rootFilepath << std::endl;
			// Load cubemap
			unsigned int textureID;
			glGenTextures(1, &textureID);
			glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

			std::string faces[6] =
			{
				rootFilepath + "/right" + extension,
				rootFilepath + "/left" + extension,
				rootFilepath + "/top" + extension,
				rootFilepath + "/bottom" + extension,
				rootFilepath + "/front" + extension,
				rootFilepath + "/back" + extension,
			};

			int width, height, nrChannels;
			for (unsigned int i = 0; i < 6; i++) {
				std::cout << "RESOURCEMANAGER::Loading cubemap face " << faces[i] << std::endl;
				unsigned int s = faces->size();
				unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);

				if (data) {
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
				}
				else {
					std::cout << "Cubemap failed to load at path: " << faces[i] << std::endl;
				}
				stbi_image_free(data);
			}
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

			Cubemap* cubemap = new Cubemap();
			cubemap->id = textureID;
			cubemap->rootFilepath = rootFilepath;

			if (loadInPersistentResources) {
				persistentResources.cubemaps[rootFilepath] = cubemap;
				return persistentResources.cubemaps[rootFilepath];
			}
			else {
				tempResources.cubemaps[rootFilepath] = cubemap;
				return tempResources.cubemaps[rootFilepath];
			}
		}

		std::unordered_map<std::string, Cubemap*>::iterator it = persistentIt;
		if (existsInTemp) { it = tempIt; }
		return it->second;
	}

	HDREnvironment* ResourceManager::LoadHDREnvironmentMap(std::string filepath, bool flipVertically, bool skipConversionAndBRDFLutGeneration, bool loadInPersistentResources)
	{
		// First check if already loaded
		std::unordered_map<std::string, HDREnvironment*>::iterator persistentIt = persistentResources.hdrCubemaps.find(filepath);
		std::unordered_map<std::string, HDREnvironment*>::iterator tempIt = tempResources.hdrCubemaps.find(filepath);

		bool existsInPersistent = true;
		bool existsInTemp = true;
		if (persistentIt == persistentResources.hdrCubemaps.end()) {
			existsInPersistent = false;
		}
		if (tempIt == tempResources.hdrCubemaps.end()) {
			existsInTemp = false;
		}

		if (!existsInPersistent && !existsInTemp) {
			std::cout << "RESOURCEMANAGER::Loading HDRI " << filepath << std::endl;
			// Load cubemap
			stbi_set_flip_vertically_on_load(flipVertically);

			unsigned int hdrTexture;
			glGenTextures(1, &hdrTexture);
			int width, height, nrComponents;
			float* data = stbi_loadf(filepath.c_str(), &width, &height, &nrComponents, 0);
			if (data) {
				glBindTexture(GL_TEXTURE_2D, hdrTexture);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			}
			else {
				std::cout << "HDR texture failed to load at path: " << filepath << std::endl;
			}
			stbi_image_free(data);
			stbi_set_flip_vertically_on_load(false);

			unsigned int irradianceMap = 0;
			unsigned int environmentMap = 0;
			unsigned int prefilterMap = 0;
			if (!skipConversionAndBRDFLutGeneration) {
				RenderManager* renderInstance = RenderManager::GetInstance();
				environmentMap = renderInstance->ConvertHDREquirectangularToCube(hdrTexture);
				irradianceMap = renderInstance->CreateIrradianceMap();
				renderInstance->ConvoluteEnvironmentMap(environmentMap, irradianceMap);
				prefilterMap = renderInstance->CreatePrefilterMap(environmentMap);
			}

			HDREnvironment* cubemap = new HDREnvironment();
			cubemap->cubemapID = environmentMap;
			cubemap->irradianceID = irradianceMap;
			cubemap->prefilterID = prefilterMap;
			cubemap->filepath = filepath;

			if (loadInPersistentResources) {
				persistentResources.hdrCubemaps[filepath] = cubemap;
				return persistentResources.hdrCubemaps[filepath];
			}
			else {
				tempResources.hdrCubemaps[filepath] = cubemap;
				return tempResources.hdrCubemaps[filepath];
			}
		}

		std::unordered_map<std::string, HDREnvironment*>::iterator it = persistentIt;
		if (existsInTemp) { it = tempIt; }
		return it->second;
	}

	TextFont* ResourceManager::LoadTextFont(std::string filepath, bool loadInPersistentResources)
	{
		// First check if already loaded
		std::unordered_map<std::string, TextFont*>::iterator persistentIt = persistentResources.textFonts.find(filepath);
		std::unordered_map<std::string, TextFont*>::iterator tempIt = tempResources.textFonts.find(filepath);

		bool existsInPersistent = true;
		bool existsInTemp = true;
		if (persistentIt == persistentResources.textFonts.end()) {
			existsInPersistent = false;
		}
		if (tempIt == tempResources.textFonts.end()) {
			existsInTemp = false;
		}

		if (!existsInPersistent && !existsInTemp) {
			std::cout << "RESOURCEMANAGER::Loading font " << filepath << std::endl;

			// Load font
			FT_Face face;
			if (FT_New_Face(freetypeLib, filepath.c_str(), 0, &face)) {
				std::cout << "Text font failed to load at path: " << filepath << std::endl;
				return nullptr;
			}

			TextFont* font = new TextFont();

			FT_Set_Pixel_Sizes(face, 0, 48 * 3);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // no byte-alignment restriction
			glEnable(GL_BLEND);
			for (unsigned char c = 0; c < 128; c++) {
				// load glyph
				if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
					std::cout << "ERROR::FREETYPE::Failed to load glyph || Filepath: " << filepath << " || c = " << c << std::endl;
					continue;
				}

				// generate glyph texture
				unsigned int texture;
				glGenTextures(1, &texture);
				glBindTexture(GL_TEXTURE_2D, texture);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				TextCharacter character = {
					texture,
					glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
					glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
					face->glyph->advance.x
				};
				font->AddCharacter(c, character);
			}
			FT_Done_Face(face);

			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
			glDisable(GL_BLEND);

			if (loadInPersistentResources) {
				persistentResources.textFonts[filepath] = font;
				return persistentResources.textFonts[filepath];
			}
			else {
				tempResources.textFonts[filepath] = font;
				return tempResources.textFonts[filepath];
			}
		}

		std::unordered_map<std::string, TextFont*>::iterator it = persistentIt;
		if (existsInTemp) { it = tempIt; }
		return it->second;
	}

	SkeletalAnimation* ResourceManager::LoadAnimation(std::string filepath, int fileAnimationIndex, bool loadInPersistentResources)
	{
		std::string indexedFilepath = filepath + "/(" + std::to_string(fileAnimationIndex) + ")";

		// First check if already loaded
		std::unordered_map<std::string, SkeletalAnimation*>::iterator persistentIt = persistentResources.animations.find(indexedFilepath);
		std::unordered_map<std::string, SkeletalAnimation*>::iterator tempIt = tempResources.animations.find(indexedFilepath);

		bool existsInPersistent = true;
		bool existsInTemp = true;
		if (persistentIt == persistentResources.animations.end()) {
			existsInPersistent = false;
		}
		if (tempIt == tempResources.animations.end()) {
			existsInTemp = false;
		}

		if (!existsInPersistent && !existsInTemp) {
			// Load animation
			std::cout << "RESOURCEMANAGER::Loading animation " << indexedFilepath << std::endl;

			Assimp::Importer importer;
			const aiScene* scene = importer.ReadFile(filepath, aiProcess_Triangulate);
			if (scene && scene->mRootNode) {
				if (fileAnimationIndex > scene->mNumAnimations - 1) {
					std::cout << "ERROR::RESOURCEMANAGER::LoadAnimation::Animation index higher than animation count in file || Index = " << fileAnimationIndex << " || Filepath: " << filepath << std::endl;
					return nullptr;
				}

				if (scene->mNumAnimations > 0) {

					aiAnimation* aiAnimation = scene->mAnimations[fileAnimationIndex];
					float duration = aiAnimation->mDuration;
					float ticksPerSecond = aiAnimation->mTicksPerSecond;

					int numChannels = aiAnimation->mNumChannels;
					std::vector<AnimationChannel> channels;
					channels.reserve(numChannels);

					for (int i = 0; i < numChannels; i++) {
						aiNodeAnim* aiChannel = aiAnimation->mChannels[i];

						std::string channelName = aiChannel->mNodeName.C_Str();

						// Position keyframes
						std::vector<AnimKeyPosition> positions;
						int numPositions = aiChannel->mNumPositionKeys;
						positions.reserve(numPositions);
						for (int j = 0; j < numPositions; j++) {
							aiVector3D aiPosition = aiChannel->mPositionKeys[j].mValue;
							float timeStamp = aiChannel->mPositionKeys[j].mTime;

							AnimKeyPosition keyframe;
							keyframe.position = glm::vec3(aiPosition.x, aiPosition.y, aiPosition.z);
							keyframe.timeStamp = timeStamp;
							positions.push_back(keyframe);
						}

						// Rotation keyframes
						std::vector<AnimKeyRotation> rotations;
						int numRotations = aiChannel->mNumRotationKeys;;
						rotations.reserve(numRotations);
						for (int j = 0; j < numRotations; j++) {
							aiQuaternion aiOrientation = aiChannel->mRotationKeys[j].mValue;
							float timeStamp = aiChannel->mRotationKeys[j].mTime;

							AnimKeyRotation keyframe;
							keyframe.orientation = glm::quat(aiOrientation.w, aiOrientation.x, aiOrientation.y, aiOrientation.z);
							keyframe.timeStamp = timeStamp;
							rotations.push_back(keyframe);
						}

						// Scale keyframes
						std::vector<AnimKeyScale> scalings;
						int numScalings = aiChannel->mNumScalingKeys;
						scalings.reserve(numScalings);
						for (int j = 0; j < numScalings; j++) {
							aiVector3D aiScale = aiChannel->mScalingKeys[j].mValue;
							float timeStamp = aiChannel->mScalingKeys[j].mTime;

							AnimKeyScale keyframe;
							keyframe.scale = glm::vec3(aiScale.x, aiScale.y, aiScale.z);
							keyframe.timeStamp = timeStamp;
							scalings.push_back(keyframe);
						}

						channels.push_back(AnimationChannel(channelName, i, positions, rotations, scalings));
					}

					SkeletalAnimation* animation = new SkeletalAnimation(channels, duration, ticksPerSecond);

					if (loadInPersistentResources) {
						persistentResources.animations[indexedFilepath] = animation;
						return persistentResources.animations[indexedFilepath];
					}
					else {
						tempResources.animations[indexedFilepath] = animation;
						return tempResources.animations[indexedFilepath];
					}
				}
				else {
					std::cout << "ERROR::RESOURCEMANAGER::LoadAnimation::Error reading animation file || Index = " << fileAnimationIndex << " || Filepath: " << filepath << std::endl;
					return nullptr;
				}
			}
			else {
				std::cout << "ERROR::RESOURCEMANAGER::LoadAnimation::Error reading animation file || Index = " << fileAnimationIndex << " || Filepath: " << filepath << std::endl;
				return nullptr;
			}
		}

		std::unordered_map<std::string, SkeletalAnimation*>::iterator it = persistentIt;
		if (existsInTemp) { it = tempIt; }
		return it->second;
	}

	AudioFile* ResourceManager::LoadAudio(std::string filepath, float defaultVolume, float defaultPan, float defaultMinAttenuationDistance, float defaultMaxAttenuationDistance, bool loadInPersistentResources)
	{
		std::unordered_map<std::string, AudioFile*>::iterator persistentIt = persistentResources.audioFiles.find(filepath);
		std::unordered_map<std::string, AudioFile*>::iterator tempIt = tempResources.audioFiles.find(filepath);

		bool existsInPersistent = true;
		bool existsInTemp = true;
		if (persistentIt == persistentResources.audioFiles.end()) {
			existsInPersistent = false;
		}
		if (tempIt == tempResources.audioFiles.end()) {
			existsInTemp = false;
		}

		if (!existsInPersistent && !existsInTemp) {
			std::cout << "RESOURCEMANAGER::Loading audio file " << filepath << std::endl;
			// Audio not currently loaded

			irrklang::ISoundSource* newSource = AudioManager::GetInstance()->GetSoundEngine()->addSoundSourceFromFile(filepath.c_str(), irrklang::ESM_NO_STREAMING, true);

			if (!newSource) {
				std::cout << "ERROR::RESOURCEMANAGER::Failed to load audio file at path: " << filepath << std::endl;
				return nullptr;
			}

			AudioFile* newAudio = new AudioFile(newSource, defaultVolume, defaultPan, defaultMinAttenuationDistance, defaultMaxAttenuationDistance);

			if (loadInPersistentResources) {
				persistentResources.audioFiles[filepath] = newAudio;
				return persistentResources.audioFiles[filepath];
			}
			else {
				tempResources.audioFiles[filepath] = newAudio;
				return tempResources.audioFiles[filepath];
			}
		}

		std::unordered_map<std::string, AudioFile*>::iterator it = persistentIt;
		if (existsInTemp) { it = tempIt; }
		return it->second;
	}

	AbstractMaterial* ResourceManager::GetMaterial(const std::string& materialName)
	{
		std::unordered_map<std::string, AbstractMaterial*>::iterator persistentIt = persistentResources.materials.find(materialName);
		std::unordered_map<std::string, AbstractMaterial*>::iterator tempIt = tempResources.materials.find(materialName);

		if (persistentIt != persistentResources.materials.end()) {
			return persistentIt->second;
		}
		else if (tempIt != tempResources.materials.end()) {
			return tempIt->second;
		}
		else {
			return nullptr;
		}
	}

	AnimationSkeleton* ResourceManager::GetAnimationSkeleton(const std::string& filename)
	{
		std::unordered_map<std::string, AnimationSkeleton*>::iterator persistentIt = persistentResources.animationSkeletons.find(filename);
		std::unordered_map<std::string, AnimationSkeleton*>::iterator tempIt = tempResources.animationSkeletons.find(filename);

		if (persistentIt != persistentResources.animationSkeletons.end()) {
			return persistentIt->second;
		}
		else if (tempIt != tempResources.animationSkeletons.end()) {
			return tempIt->second;
		}
		else {
			return nullptr;
		}
	}

	MeshData* ResourceManager::GetMeshData(const std::string& fileNamePlusMeshName)
	{
		std::unordered_map<std::string, MeshData*>::iterator persistentIt = persistentResources.meshes.find(fileNamePlusMeshName);
		std::unordered_map<std::string, MeshData*>::iterator tempIt = tempResources.meshes.find(fileNamePlusMeshName);

		if (persistentIt != persistentResources.meshes.end()) {
			return persistentIt->second;
		}
		else if (tempIt != tempResources.meshes.end()) {
			return tempIt->second;
		}
		else {
			return nullptr;
		}
	}

	ComputeShader* ResourceManager::GetComputeShader(const char* computePath)
	{
		std::unordered_map<std::string, AbstractShader*>::iterator persistentIt = persistentResources.shaders.find(computePath);
		std::unordered_map<std::string, AbstractShader*>::iterator tempIt = tempResources.shaders.find(computePath);

		if (persistentIt != persistentResources.shaders.end()) {
			return dynamic_cast<ComputeShader*>(persistentIt->second);
		}
		else if (tempIt != tempResources.shaders.end()) {
			return dynamic_cast<ComputeShader*>(tempIt->second);
		}
		else {
			return nullptr;
		}
	}

	Shader* ResourceManager::GetShader(const std::string& vertexPath, const std::string& fragmentPath)
	{
		std::string combinedPath = vertexPath + "|" + fragmentPath;

		std::unordered_map<std::string, AbstractShader*>::iterator persistentIt = persistentResources.shaders.find(combinedPath);
		std::unordered_map<std::string, AbstractShader*>::iterator tempIt = tempResources.shaders.find(combinedPath);

		if (persistentIt != persistentResources.shaders.end()) {
			return dynamic_cast<Shader*>(persistentIt->second);
		}
		else if (tempIt != tempResources.shaders.end()) {
			return dynamic_cast<Shader*>(tempIt->second);
		}
		else {
			return nullptr;
		}
	}

	Shader* ResourceManager::GetShader(const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath)
	{
		std::string combinedPath = vertexPath + "|" + fragmentPath + "|" + geometryPath;

		std::unordered_map<std::string, AbstractShader*>::iterator persistentIt = persistentResources.shaders.find(combinedPath);
		std::unordered_map<std::string, AbstractShader*>::iterator tempIt = tempResources.shaders.find(combinedPath);

		if (persistentIt != persistentResources.shaders.end()) {
			return dynamic_cast<Shader*>(persistentIt->second);
		}
		else if (tempIt != tempResources.shaders.end()) {
			return dynamic_cast<Shader*>(tempIt->second);
		}
		else {
			return nullptr;
		}
	}

	Texture* ResourceManager::GetTexture(const std::string& filepath)
	{
		std::unordered_map<std::string, Texture*>::iterator persistentIt = persistentResources.textures.find(filepath);
		std::unordered_map<std::string, Texture*>::iterator tempIt = tempResources.textures.find(filepath);

		if (persistentIt != persistentResources.textures.end()) {
			return persistentIt->second;
		}
		else if (tempIt != tempResources.textures.end()) {
			return tempIt->second;
		}
		else {
			return nullptr;
		}
	}

	Cubemap* ResourceManager::GetCubemap(const std::string& filepath)
	{
		std::unordered_map<std::string, Cubemap*>::iterator persistentIt = persistentResources.cubemaps.find(filepath);
		std::unordered_map<std::string, Cubemap*>::iterator tempIt = tempResources.cubemaps.find(filepath);

		if (persistentIt != persistentResources.cubemaps.end()) {
			return persistentIt->second;
		}
		else if (tempIt != tempResources.cubemaps.end()) {
			return tempIt->second;
		}
		else {
			return nullptr;
		}
	}

	HDREnvironment* ResourceManager::GetHDREnvironmentMap(const std::string& filepath)
	{
		std::unordered_map<std::string, HDREnvironment*>::iterator persistentIt = persistentResources.hdrCubemaps.find(filepath);
		std::unordered_map<std::string, HDREnvironment*>::iterator tempIt = tempResources.hdrCubemaps.find(filepath);

		if (persistentIt != persistentResources.hdrCubemaps.end()) {
			return persistentIt->second;
		}
		else if (tempIt != tempResources.hdrCubemaps.end()) {
			return tempIt->second;
		}
		else {
			return nullptr;
		}
	}

	TextFont* ResourceManager::GetTextFont(const std::string& filepath)
	{
		std::unordered_map<std::string, TextFont*>::iterator persistentIt = persistentResources.textFonts.find(filepath);
		std::unordered_map<std::string, TextFont*>::iterator tempIt = tempResources.textFonts.find(filepath);

		if (persistentIt != persistentResources.textFonts.end()) {
			return persistentIt->second;
		}
		else if (tempIt != tempResources.textFonts.end()) {
			return tempIt->second;
		}
		else {
			return nullptr;
		}
	}

	SkeletalAnimation* ResourceManager::GetAnimation(const std::string& filepath, const int animationIndex)
	{
		std::string indexedFilepath = filepath + "/(" + std::to_string(animationIndex) + ")";

		std::unordered_map<std::string, SkeletalAnimation*>::iterator persistentIt = persistentResources.animations.find(indexedFilepath);
		std::unordered_map<std::string, SkeletalAnimation*>::iterator tempIt = tempResources.animations.find(indexedFilepath);

		if (persistentIt != persistentResources.animations.end()) {
			return persistentIt->second;
		}
		else if (tempIt != tempResources.animations.end()) {
			return tempIt->second;
		}
		else {
			return nullptr;
		}
	}

	AudioFile* ResourceManager::GetAudio(const std::string& filepath)
	{
		std::unordered_map<std::string, AudioFile*>::iterator persistentIt = persistentResources.audioFiles.find(filepath);
		std::unordered_map<std::string, AudioFile*>::iterator tempIt = tempResources.audioFiles.find(filepath);

		if (persistentIt != persistentResources.audioFiles.end()) {
			return persistentIt->second;
		}
		else if (tempIt != tempResources.audioFiles.end()) {
			return tempIt->second;
		}
		else {
			return nullptr;
		}
	}

	Material* ResourceManager::GenerateMaterial(std::vector<Texture*> diffuseMaps, std::vector<Texture*> specularMaps, std::vector<Texture*> normalMaps, std::vector<Texture*> heightMaps, float shininess, glm::vec3 diffuse, glm::vec3 specular)
	{
		Material* material = new Material();
		material->baseColourMaps = diffuseMaps;
		material->specularMaps = specularMaps;
		material->normalMaps = normalMaps;
		material->heightMaps = heightMaps;
		material->shininess = shininess;
		material->baseColour = diffuse;
		material->specular = specular;
		return material;
	}
}