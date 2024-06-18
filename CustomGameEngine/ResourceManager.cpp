#include "ResourceManager.h"
#include "RenderManager.h"
#include "AudioManager.h"
namespace Engine {

	ResourceManager* ResourceManager::instance = nullptr;
	ResourceManager::ResourceManager()
	{
		// Setup freetype
		if (FT_Init_FreeType(&freetypeLib)) {
			std::cout << "FAIL::RESOURCEMANAGER::Failed to initialize FreeType Library" << std::endl;
		}

		// Load defaults
		defaultMaterial = new Material();
		defaultMaterial->diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
		defaultMaterial->specular = glm::vec3(1.0f, 1.0f, 1.0f);
		defaultMaterial->shininess = 105.0f;
		//defaultMaterial->shininess = 60.0f;

		defaultMaterialPBR = new PBRMaterial();
		defaultMaterialPBR->albedo = glm::vec3(1.0f, 1.0f, 1.0f);
		defaultMaterialPBR->metallic = 0.0f;
		defaultMaterialPBR->roughness = 0.5f;
		defaultMaterialPBR->ao = 1.0f;
		defaultMaterialPBR->isTransparent = false;

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
		defaultCube = new Mesh(vertices, indices, defaultMaterial);
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
		defaultPlane = new Mesh(vertices, indices, defaultMaterial);
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
		defaultSphere = new Mesh(vertices, indices, defaultMaterial);
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

		int textureOffset = 18;

		shadowMapShader = LoadShader("Shaders/depthMap.vert", "Shaders/depthMap.frag");
		cubeShadowMapShader = LoadShader("Shaders/cubeDepthMap.vert", "Shaders/cubeDepthMap.frag", "Shaders/cubeDepthMap.geom");
		defaultLitShader = LoadShader("Shaders/defaultLitNew.vert", "Shaders/defaultLitNew.frag");
		screenQuadShader = LoadShader("Shaders/screenQuad.vert", "Shaders/screenQuad.frag");
		hdrTonemappingShader = LoadShader("Shaders/screenQuad.vert", "Shaders/hdrTonemapping.frag");
		deferredGeometryPass = LoadShader("Shaders/g_buffer.vert", "Shaders/g_buffer.frag");
		deferredGeometryPassPBR = LoadShader("Shaders/g_buffer.vert", "Shaders/g_bufferPBR.frag");
		deferredLightingPass = LoadShader("Shaders/defaultDeferred.vert", "Shaders/defaultDeferred.frag");
		deferredLightingPassPBR = LoadShader("Shaders/defaultDeferred.vert", "Shaders/defaultDeferredPBR.frag");
		ssaoShader = LoadShader("Shaders/ssao.vert", "Shaders/ssao.frag");
		ssaoBlur = LoadShader("Shaders/ssao.vert", "Shaders/ssaoBlur.frag");
		skyboxShader = LoadShader("Shaders/skybox.vert", "Shaders/skybox.frag");
		defaultLitPBRShader = LoadShader("Shaders/defaultLit_pbr.vert", "Shaders/defaultLit_pbr.frag");
		bloomBlur = LoadShader("Shaders/bloomBlur.vert", "Shaders/bloomBlur.frag");
		equirectangularToCubemapShader = LoadShader("Shaders/convertToCubemap.vert", "Shaders/convertToCubemap.frag");
		createIrradianceShader = LoadShader("Shaders/convertToCubemap.vert", "Shaders/irradianceConvolute.frag");
		createPrefilterShader = LoadShader("Shaders/convertToCubemap.vert", "Shaders/prefilter.frag");
		createBRDFShader = LoadShader("Shaders/brdf.vert", "Shaders/brdf.frag");
		defaultTextShader = LoadShader("Shaders/defaultText.vert", "Shaders/defaultText.frag");
		defaultImageShader = LoadShader("Shaders/defaultImage.vert", "Shaders/defaultImage.frag");
		particleShader = LoadShader("Shaders/particles.vert", "Shaders/particles.frag");
		pointParticleShader = LoadShader("Shaders/pointParticles.vert", "Shaders/particles.frag", "Shaders/pointParticles.geom");
		advBloomDownsampleShader = LoadShader("Shaders/screenQuad.vert", "Shaders/advBloomDownsample.frag");
		advBloomUpsampleShader = LoadShader("Shaders/screenQuad.vert", "Shaders/advBloomUpsample.frag");

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

		defaultLitShader->Use();
		defaultLitShader->setInt("dirLight.ShadowMap", 0);
		for (int i = 0; i <= 8; i++) {
			defaultLitShader->setInt((std::string("lights[" + std::string(std::to_string(i)) + std::string("].ShadowMap"))), i + 1);
			defaultLitShader->setInt((std::string("lights[" + std::string(std::to_string(i)) + std::string("].CubeShadowMap"))), i + 8 + 1);
		}

		defaultLitShader->setInt("material.TEXTURE_DIFFUSE1", 1 + textureOffset);
		defaultLitShader->setInt("material.TEXTURE_SPECULAR1", 2 + textureOffset);
		defaultLitShader->setInt("material.TEXTURE_NORMAL1", 3 + textureOffset);
		defaultLitShader->setInt("material.TEXTURE_DISPLACE1", 4 + textureOffset);
		defaultLitShader->setInt("material.TEXTURE_OPACITY1", 5 + textureOffset);

		deferredGeometryPass->Use();
		deferredGeometryPass->setInt("material.TEXTURE_DIFFUSE1", 1 + textureOffset);
		deferredGeometryPass->setInt("material.TEXTURE_SPECULAR1", 2 + textureOffset);
		deferredGeometryPass->setInt("material.TEXTURE_NORMAL1", 3 + textureOffset);
		deferredGeometryPass->setInt("material.TEXTURE_DISPLACE1", 4 + textureOffset);
		deferredGeometryPass->setInt("material.TEXTURE_OPACITY1", 5 + textureOffset);

		deferredGeometryPassPBR->Use();
		deferredGeometryPassPBR->setInt("material.TEXTURE_ALBEDO1", 1 + textureOffset);
		deferredGeometryPassPBR->setInt("material.TEXTURE_NORMAL1", 2 + textureOffset);
		deferredGeometryPassPBR->setInt("material.TEXTURE_METALLIC1", 3 + textureOffset);
		deferredGeometryPassPBR->setInt("material.TEXTURE_ROUGHNESS1", 4 + textureOffset);
		deferredGeometryPassPBR->setInt("material.TEXTURE_AO1", 5 + textureOffset);
		deferredGeometryPassPBR->setInt("material.TEXTURE_DISPLACE1", 6 + textureOffset);

		deferredLightingPass->Use();
		deferredLightingPass->setInt("dirLight.ShadowMap", 0);
		for (int i = 0; i <= 8; i++) {
			deferredLightingPass->setInt((std::string("lights[" + std::string(std::to_string(i)) + std::string("].ShadowMap"))), i + 1);
			deferredLightingPass->setInt((std::string("lights[" + std::string(std::to_string(i)) + std::string("].CubeShadowMap"))), i + 8 + 1);
		}

		deferredLightingPass->setInt("gPosition", 18);
		deferredLightingPass->setInt("gNormal", 19);
		deferredLightingPass->setInt("gAlbedo", 20);
		deferredLightingPass->setInt("gSpecular", 21);
		deferredLightingPass->setInt("gPBRFLAG", 22);
		deferredLightingPass->setInt("SSAO", 23);

		deferredLightingPassPBR->Use();
		deferredLightingPassPBR->setInt("dirLight.ShadowMap", 0);
		for (int i = 0; i <= 8; i++) {
			deferredLightingPassPBR->setInt((std::string("lights[" + std::string(std::to_string(i)) + std::string("].ShadowMap"))), i + 1);
			deferredLightingPassPBR->setInt((std::string("lights[" + std::string(std::to_string(i)) + std::string("].CubeShadowMap"))), i + 8 + 1);
		}

		deferredLightingPassPBR->setInt("gPosition", 18);
		deferredLightingPassPBR->setInt("gNormal", 19);
		deferredLightingPassPBR->setInt("gAlbedo", 20);
		deferredLightingPassPBR->setInt("gArm", 21);
		deferredLightingPassPBR->setInt("gPBRFLAG", 22);
		deferredLightingPassPBR->setInt("SSAO", 23);

		deferredLightingPassPBR->setInt("irradianceMap", 9 + textureOffset);
		deferredLightingPassPBR->setInt("prefilterMap", 10 + textureOffset);
		deferredLightingPassPBR->setInt("brdfLUT", 11 + textureOffset);

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

		defaultLitPBRShader->setInt("dirLight.ShadowMap", 0);
		for (int i = 0; i <= 8; i++) {
			defaultLitPBRShader->setInt((std::string("lights[" + std::string(std::to_string(i)) + std::string("].ShadowMap"))), i + 1);
			defaultLitPBRShader->setInt((std::string("lights[" + std::string(std::to_string(i)) + std::string("].CubeShadowMap"))), i + 8 + 1);
		}

		defaultLitPBRShader->setInt("material.TEXTURE_ALBEDO1", 1 + textureOffset);
		defaultLitPBRShader->setInt("material.TEXTURE_NORMAL1", 2 + textureOffset);
		defaultLitPBRShader->setInt("material.TEXTURE_METALLIC1", 3 + textureOffset);
		defaultLitPBRShader->setInt("material.TEXTURE_ROUGHNESS1", 4 + textureOffset);
		defaultLitPBRShader->setInt("material.TEXTURE_AO1", 5 + textureOffset);
		defaultLitPBRShader->setInt("material.TEXTURE_DISPLACE1", 6 + textureOffset);
		defaultLitPBRShader->setInt("material.TEXTURE_OPACITY1", 7 + textureOffset);
		defaultLitPBRShader->setInt("irradianceMap", 9 + textureOffset);
		defaultLitPBRShader->setInt("prefilterMap", 10 + textureOffset);
		defaultLitPBRShader->setInt("brdfLUT", 11 + textureOffset);

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

		// delete models
		std::unordered_map<std::string, Model*>::iterator modelsIt = models.begin();
		while (modelsIt != models.end()) {
			delete modelsIt->second;
			modelsIt++;
		}

		// delete shaders
		std::unordered_map<std::string, Shader*>::iterator shadersIt = shaders.begin();
		while (shadersIt != shaders.end()) {
			delete shadersIt->second;
			shadersIt++;
		}

		// delete textures
		std::unordered_map<std::string, Texture*>::iterator texturesIt = textures.begin();
		while (texturesIt != textures.end()) {
			delete texturesIt->second;
			texturesIt++;
		}

		// delete cubemaps
		std::unordered_map<std::string, Cubemap*>::iterator cubemapsIt = cubemaps.begin();
		while (cubemapsIt != cubemaps.end()) {
			delete cubemapsIt->second;
			cubemapsIt++;
		}

		// delete hdr cubemaps
		std::unordered_map<std::string, HDREnvironment*>::iterator hdrmapsIt = hdrCubemaps.begin();
		while (hdrmapsIt != hdrCubemaps.end()) {
			delete hdrmapsIt->second;
			hdrmapsIt++;
		}

		// delete fonts
		std::unordered_map<std::string, TextFont*>::iterator textFontsIt = textFonts.begin();
		while (textFontsIt != textFonts.end()) {
			delete textFontsIt->second;
			textFontsIt++;
		}

		// delete animations
		std::unordered_map<std::string, SkeletalAnimation*>::iterator animationsIt = animations.begin();
		while (animationsIt != animations.end()) {
			delete animationsIt->second;
			animationsIt++;
		}

		// delete audio files
		std::unordered_map<std::string, AudioFile*>::iterator audioIt = audioFiles.begin();
		while (audioIt != audioFiles.end()) {
			delete audioIt->second;
			audioIt++;
		}

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

	Model* ResourceManager::LoadModel(std::string filepath, bool pbr)
	{
		std::unordered_map<std::string, Model*>::iterator it = models.find(filepath);

		if (it == models.end()) {
			std::cout << "RESOURCEMANAGER::Loading model " << filepath << std::endl;
			// Model not currently loaded
			models[filepath] = new Model(filepath.c_str(), pbr);
			return models[filepath];
		}
		else {
			if (it->second->PBR() == pbr) {
				return it->second;
			}
			else {
				std::cout << "ERROR::RESOURCEMANAGER::LOADMODEL::Mismatch, requested model at path '" << filepath << "' already loaded with different PBR property" << std::endl;
				return nullptr;
			}
		}
	}

	Shader* ResourceManager::LoadShader(std::string vertexPath, std::string fragmentPath)
	{		
		std::string combinedPath = vertexPath + "|" + fragmentPath;

		std::unordered_map<std::string, Shader*>::iterator it = shaders.find(combinedPath);

		if (it == shaders.end()) {
			std::cout << "RESOURCEMANAGER::Loading shader " << combinedPath << std::endl;
			// Shader not currently loaded
			shaders[combinedPath] = new Shader(vertexPath.c_str(), fragmentPath.c_str());
			return shaders[combinedPath];
		}

		return it->second;
	}

	Shader* ResourceManager::LoadShader(std::string vertexPath, std::string fragmentPath, std::string geometryPath)
	{
		std::string combinedPath = vertexPath + "|" + fragmentPath + "|" + geometryPath;

		std::unordered_map<std::string, Shader*>::iterator it = shaders.find(combinedPath);

		if (it == shaders.end()) {
			std::cout << "RESOURCEMANAGER::Loading shader " << combinedPath << std::endl;
			// Shader not currently loaded
			shaders[combinedPath] = new Shader(vertexPath.c_str(), fragmentPath.c_str(), geometryPath.c_str());
			return shaders[combinedPath];
		}

		return it->second;
	}

	Texture* ResourceManager::LoadTexture(std::string filepath, TextureTypes type, bool srgb)
	{
		// First check if already loaded
		std::unordered_map<std::string, Texture*>::iterator it = textures.find(filepath);

		if (it == textures.end()) {
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

			textures[filepath] = texture;
			return textures[filepath];
		}

		return it->second;
	}

	Cubemap* ResourceManager::LoadCubemap(std::string rootFilepath)
	{
		// First check if already loaded
		std::unordered_map<std::string, Cubemap*>::iterator it = cubemaps.find(rootFilepath);

		if (it == cubemaps.end()) {
			std::cout << "RESOURCEMANAGER::Loading cubemap root " << rootFilepath << std::endl;
			// Load cubemap
			unsigned int textureID;
			glGenTextures(1, &textureID);
			glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

			std::string faces[6] =
			{
				rootFilepath + "/right.png",
				rootFilepath + "/left.png",
				rootFilepath + "/top.png",
				rootFilepath + "/bottom.png",
				rootFilepath + "/front.png",
				rootFilepath + "/back.png"
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
			cubemaps[rootFilepath] = cubemap;
			return cubemaps[rootFilepath];
		}

		return it->second;
	}

	HDREnvironment* ResourceManager::LoadHDREnvironmentMap(std::string filepath, bool flipVertically, bool skipConversionAndBRDFLutGeneration)
	{
		// First check if already loaded
		std::unordered_map<std::string, HDREnvironment*>::iterator it = hdrCubemaps.find(filepath);

		if (it == hdrCubemaps.end()) {
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
			unsigned int brdf_LUT = 0;
			unsigned int environmentMap = 0;
			unsigned int prefilterMap = 0;
			if (!skipConversionAndBRDFLutGeneration) {
				RenderManager* renderInstance = RenderManager::GetInstance();
				environmentMap = renderInstance->ConvertHDREquirectangularToCube(hdrTexture);
				irradianceMap = renderInstance->CreateIrradianceMap();
				renderInstance->ConvoluteEnvironmentMap(environmentMap, irradianceMap);
				prefilterMap = renderInstance->CreatePrefilterMap(environmentMap);
				brdf_LUT = renderInstance->CreateBRDF();
			}

			HDREnvironment* cubemap = new HDREnvironment();
			cubemap->cubemapID = environmentMap;
			cubemap->irradianceID = irradianceMap;
			cubemap->brdf_lutID = brdf_LUT;
			cubemap->prefilterID = prefilterMap;
			cubemap->filepath = filepath;
			hdrCubemaps[filepath] = cubemap;
			return hdrCubemaps[filepath];
		}

		return it->second;
	}

	TextFont* ResourceManager::LoadTextFont(std::string filepath)
	{
		// First check if already loaded
		std::unordered_map<std::string, TextFont*>::iterator it = textFonts.find(filepath);

		if (it == textFonts.end()) {
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

			textFonts[filepath] = font;
			return textFonts[filepath];
		}

		return it->second;
	}

	SkeletalAnimation* ResourceManager::LoadAnimation(std::string filepath, int fileAnimationIndex)
	{
		std::string indexedFilepath = filepath + "/(" + std::to_string(fileAnimationIndex) + ")";

		// First check if already loaded
		std::unordered_map<std::string, SkeletalAnimation*>::iterator it = animations.find(indexedFilepath);

		if (it == animations.end()) {
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
					animations[indexedFilepath] = animation;
					return animations[indexedFilepath];
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

		return it->second;
	}

	AudioFile* ResourceManager::LoadAudio(std::string filepath, float defaultVolume, float defaultPan, float defaultMinAttenuationDistance, float defaultMaxAttenuationDistance)
	{
		std::unordered_map<std::string, AudioFile*>::iterator it = audioFiles.find(filepath);

		if (it == audioFiles.end()) {
			std::cout << "RESOURCEMANAGER::Loading audio file " << filepath << std::endl;
			// Audio not currently loaded

			irrklang::ISoundSource* newSource = AudioManager::GetInstance()->GetSoundEngine()->addSoundSourceFromFile(filepath.c_str(), irrklang::ESM_NO_STREAMING, true);

			if (!newSource) {
				std::cout << "ERROR::RESOURCEMANAGER::Failed to load audio file at path: " << filepath << std::endl;
				return nullptr;
			}

			AudioFile* newAudio = new AudioFile(newSource, defaultVolume, defaultPan, defaultMinAttenuationDistance, defaultMaxAttenuationDistance);
			audioFiles[filepath] = newAudio;

			return audioFiles[filepath];
		}

		return it->second;
	}

	Material* ResourceManager::GenerateMaterial(std::vector<Texture*> diffuseMaps, std::vector<Texture*> specularMaps, std::vector<Texture*> normalMaps, std::vector<Texture*> heightMaps, float shininess, glm::vec3 diffuse, glm::vec3 specular)
	{
		Material* material = new Material();
		material->diffuseMaps = diffuseMaps;
		material->specularMaps = specularMaps;
		material->normalMaps = normalMaps;
		material->heightMaps = heightMaps;
		material->shininess = shininess;
		material->diffuse = diffuse;
		material->specular = specular;
		return material;
	}
}