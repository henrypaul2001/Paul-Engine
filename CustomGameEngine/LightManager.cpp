#include "LightManager.h"
#include "ComponentLight.h"
#include "ComponentTransform.h"
#include "RenderManager.h"
namespace Engine {
	LightManager* LightManager::instance = nullptr;
	LightManager::LightManager() {
		directionalLight = nullptr;
		lightEntities = std::vector<Entity*>();
		textureSlots = &ResourceManager::GetInstance()->GetTextureSlotLookupMap();
	}

	LightManager::~LightManager() {
		delete instance;
	}

	LightManager* LightManager::GetInstance()
	{
		if (instance == nullptr) {
			instance = new LightManager();
		}

		return instance;
	}

	void LightManager::AddLightEntity(Entity* entity)
	{
		//ComponentLight* lightComponent = dynamic_cast<ComponentLight*>(entity->GetComponent(COMPONENT_LIGHT));
		lightEntities.push_back(entity);
		//DepthMapType type = lightComponent->GetLightType() == POINT ? MAP_CUBE : MAP_2D;
		//RenderManager::GetInstance(1024, 1024)->AssignLightToShadowMap(lightEntities.size() - 1, type);
	}

	void LightManager::SetDirectionalLightEntity(Entity* entity)
	{
		directionalLight = entity;
	}

	void LightManager::RemoveLightEntity(Entity* entity)
	{
		for (int i = 0; i < lightEntities.size(); i++) {
			if (lightEntities[i] == entity) {
				lightEntities.erase(lightEntities.begin() + i);
				return;
			}
		}
	}

	void LightManager::SetDirectionalLightUniforms(Shader* shader, Entity* directionalLight)
	{
		RenderManager* renderInstance = RenderManager::GetInstance();
		ComponentLight* directional = dynamic_cast<ComponentLight*>(directionalLight->GetComponent(COMPONENT_LIGHT));
		shader->setVec3("dirLight.Direction", directional->Direction);
		shader->setVec3("dirLight.Colour", directional->Colour);
		shader->setVec3("dirLight.Specular", directional->Specular);
		shader->setVec3("dirLight.Ambient", directional->Ambient);
		shader->setFloat("dirLight.LightDistance", directional->DirectionalLightDistance);

		if ((renderInstance->GetRenderParams()->GetRenderOptions() & RENDER_SHADOWS) != 0) {
			shader->setBool("dirLight.CastShadows", directional->CastShadows);
		}
		else {
			shader->setBool("dirLight.CastShadows", false);
		}
		
		shader->setFloat("dirLight.MinShadowBias", directional->MinShadowBias);
		shader->setFloat("dirLight.MaxShadowBias", directional->MaxShadowBias);
		shader->setBool("dirLight.Active", directional->Active);

		glm::vec3 lightPos = -directional->Direction * directional->DirectionalLightDistance; // negative of the directional light's direction

		glm::mat4 lightProjection = glm::ortho(-directional->ShadowProjectionSize, directional->ShadowProjectionSize, -directional->ShadowProjectionSize, directional->ShadowProjectionSize, directional->Near, directional->Far);
		glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 lightSpaceMatrix = lightProjection * lightView;

		shader->setMat4("dirLight.LightSpaceMatrix", lightSpaceMatrix);
		shader->setVec2(std::string("dirLight.shadowResolution"), glm::vec2(renderInstance->ShadowWidth(), renderInstance->ShadowHeight()));

		glActiveTexture(GL_TEXTURE0 + textureSlots->at("dirLight.ShadowMap"));
		glBindTexture(GL_TEXTURE_2D, renderInstance->GetDirectionalShadowMap());
	}

	void LightManager::SetIBLUniforms(Shader* shader, Camera* activeCamera)
	{
		RenderManager* renderManager = RenderManager::GetInstance();
		RenderOptions renderOptions = renderManager->GetRenderParams()->GetRenderOptions();
		if ((renderOptions & RENDER_ENVIRONMENT_MAP)) {
			shader->setBool("useGlobalIBL", true);
			glActiveTexture(GL_TEXTURE0 + textureSlots->at("globalIBL.irradianceMap"));
			glBindTexture(GL_TEXTURE_CUBE_MAP, renderManager->GetEnvironmentMap()->irradianceID);

			glActiveTexture(GL_TEXTURE0 + textureSlots->at("globalIBL.prefilterMap"));
			glBindTexture(GL_TEXTURE_CUBE_MAP, renderManager->GetEnvironmentMap()->prefilterID);
		}

		glActiveTexture(GL_TEXTURE0 + textureSlots->at("brdfLUT"));
		glBindTexture(GL_TEXTURE_2D, renderManager->GetGlobalBRDF_LUT());

		std::string name;
		const std::map<float, ReflectionProbe*> culledReflectionProbes = renderManager->GetBakedData().GetCulledProbeList();
		std::map<float, ReflectionProbe*>::const_iterator it = culledReflectionProbes.begin();
		int index = 0;

		while (index < 3 && it != culledReflectionProbes.end()) {
			ReflectionProbe* probe = it->second;
			ReflectionProbeEnvironmentMap envMap = probe->GetProbeEnvMap();

			name = "localIBLProbes[" + std::to_string(index) + "].irradianceMap";
			glActiveTexture(GL_TEXTURE0 + textureSlots->at(name));
			glBindTexture(GL_TEXTURE_CUBE_MAP, envMap.irradianceID);

			name = "localIBLProbes[" + std::to_string(index) + "].prefilterMap";
			glActiveTexture(GL_TEXTURE0 + textureSlots->at(name));
			glBindTexture(GL_TEXTURE_CUBE_MAP, envMap.prefilterID);

			shader->setVec3("localIBLProbes[" + std::to_string(index) + "].worldPos", probe->GetWorldPosition());
			shader->setFloat("localIBLProbes[" + std::to_string(index) + "].soiRadius", probe->GetSOIRadius());

			AABBPoints geoBounds = probe->GetLocalGeometryBounds();
			shader->setFloat("localIBLProbes[" + std::to_string(index) + "].geoApproximationAABB.minX", geoBounds.minX);
			shader->setFloat("localIBLProbes[" + std::to_string(index) + "].geoApproximationAABB.minY", geoBounds.minY);
			shader->setFloat("localIBLProbes[" + std::to_string(index) + "].geoApproximationAABB.minZ", geoBounds.minZ);

			shader->setFloat("localIBLProbes[" + std::to_string(index) + "].geoApproximationAABB.maxX", geoBounds.maxX);
			shader->setFloat("localIBLProbes[" + std::to_string(index) + "].geoApproximationAABB.maxY", geoBounds.maxY);
			shader->setFloat("localIBLProbes[" + std::to_string(index) + "].geoApproximationAABB.maxZ", geoBounds.maxZ);

			shader->setInt("activeLocalIBLProbes", index + 1);

			// Increment iterator
			it++;
			index = std::distance(culledReflectionProbes.begin(), it);
		}
	}

	void LightManager::SetShaderUniforms(Shader* shader, Camera* activeCamera)
	{
		RenderManager* renderInstance = RenderManager::GetInstance();
		const FlatTextureAtlas* spotShadowAtlas = renderInstance->GetFlatShadowmapTextureAtlas();
		//const CubeTextureAtlas* pointShadowAtlas = renderInstance->GetCubeShadowmapTextureAtlas();
		unsigned int slotRow, slotColumn;
		unsigned int numFlatShadowColumns = spotShadowAtlas->GetNumColumns();
		//unsigned int numCubeShadowColumns = pointShadowAtlas->GetNumColumns();
		shader->Use();

		// First set directional light
		if (directionalLight != nullptr) {
			SetDirectionalLightUniforms(shader, directionalLight);
		}
		else {
			shader->setBool("dirLight.Active", false);
		}

		shader->setInt("activeLights", lightEntities.size());
		if (lightEntities.size() > 0) {
			glActiveTexture(GL_TEXTURE0 + textureSlots->at("spotlightShadowAtlas"));
			glBindTexture(GL_TEXTURE_2D, spotShadowAtlas->GetTextureID());

			glActiveTexture(GL_TEXTURE0 + textureSlots->at("pointLightShadowArray"));
			glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, renderInstance->GetPointLightCubemapShadowArray());
		}

		shader->setFloat("BloomThreshold", renderInstance->GetRenderParams()->GetBloomThreshold());
		shader->setVec3("localIBLProbes[0].worldPos", glm::vec3(10.0f, 0.0f, 0.0f));

		bool globalShadows = (renderInstance->GetRenderParams()->GetRenderOptions() & RENDER_SHADOWS) != 0;
		std::string name;
		// Now spot and point lights
		for (int i = 0; i < lightEntities.size() && i < 8; i++) {
			ComponentLight* lightComponent = dynamic_cast<ComponentLight*>(lightEntities[i]->GetComponent(COMPONENT_LIGHT));
			ComponentTransform* transformComponent = dynamic_cast<ComponentTransform*>(lightEntities[i]->GetComponent(COMPONENT_TRANSFORM));

			shader->setVec3(std::string("lights[" + std::string(std::to_string(i)) + std::string("].Position")), transformComponent->GetWorldPosition());
			shader->setVec3(std::string("lights[" + std::string(std::to_string(i)) + std::string("].Colour")), lightComponent->Colour);
			shader->setVec3(std::string("lights[" + std::string(std::to_string(i)) + std::string("].Specular")), lightComponent->Specular);
			shader->setVec3(std::string("lights[" + std::string(std::to_string(i)) + std::string("].Ambient")), lightComponent->Ambient);
			shader->setFloat(std::string("lights[" + std::string(std::to_string(i)) + std::string("].Constant")), lightComponent->Constant);
			shader->setFloat(std::string("lights[" + std::string(std::to_string(i)) + std::string("].Linear")), lightComponent->Linear);
			shader->setFloat(std::string("lights[" + std::string(std::to_string(i)) + std::string("].Quadratic")), lightComponent->Quadratic);

			if (globalShadows) {
				shader->setBool(std::string("lights[" + std::string(std::to_string(i)) + std::string("].CastShadows")), lightComponent->CastShadows);
			}
			else {
				shader->setBool(std::string("lights[" + std::string(std::to_string(i)) + std::string("].CastShadows")), false);
			}

			shader->setFloat(std::string("lights[" + std::string(std::to_string(i)) + std::string("].MinShadowBias")), lightComponent->MinShadowBias);
			shader->setFloat(std::string("lights[" + std::string(std::to_string(i)) + std::string("].MaxShadowBias")), lightComponent->MaxShadowBias);
			shader->setBool(std::string("lights[" + std::string(std::to_string(i)) + std::string("].Active")), lightComponent->Active);

			if (lightComponent->GetLightType() == POINT) {
				shader->setVec2(std::string("lights[" + std::string(std::to_string(i)) + std::string("].shadowResolution")), glm::vec2(renderInstance->ShadowWidth(), renderInstance->ShadowHeight()));
				shader->setBool(std::string("lights[" + std::string(std::to_string(i)) + std::string("].SpotLight")), false);
				shader->setFloat(std::string("lights[" + std::string(std::to_string(i)) + std::string("].ShadowFarPlane")), lightComponent->Far);
			}
			else if (lightComponent->GetLightType() == SPOT) {
				// Rotate light direction based on transform matrix
				glm::mat4 model = glm::mat4(glm::mat3(transformComponent->GetWorldModelMatrix()));
				glm::vec4 rotatedDirection4 = model * glm::vec4(lightComponent->Direction, 1.0);

				glm::vec3 rotatedDirection = glm::vec3(rotatedDirection4);
				rotatedDirection = glm::normalize(rotatedDirection);
				lightComponent->WorldDirection = rotatedDirection;

				float aspect = (float)renderInstance->ShadowWidth() / (float)renderInstance->ShadowHeight();
				glm::vec3 lightPos = transformComponent->GetWorldPosition();
				glm::mat4 lightProjection = glm::perspective(glm::radians(90.0f), aspect, lightComponent->Near, lightComponent->Far);
				glm::mat4 lightView = glm::lookAt(lightPos, lightPos + lightComponent->WorldDirection, glm::vec3(0.0f, 1.0f, 0.0f));
				glm::mat4 lightSpaceMatrix = lightProjection * lightView;

				if (i < numFlatShadowColumns) {
					slotRow = 0;
					slotColumn = i;
				}
				else {
					slotRow = i / numFlatShadowColumns;
					slotColumn = i % numFlatShadowColumns;
				}

				shader->setBool(std::string("lights[" + std::string(std::to_string(i)) + std::string("].SpotLight")), true);
				shader->setVec3(std::string("lights[" + std::string(std::to_string(i)) + std::string("].Direction")), lightComponent->WorldDirection);
				shader->setFloat(std::string("lights[" + std::string(std::to_string(i)) + std::string("].Cutoff")), lightComponent->Cutoff);
				shader->setFloat(std::string("lights[" + std::string(std::to_string(i)) + std::string("].OuterCutoff")), lightComponent->OuterCutoff);
				shader->setMat4(std::string("lights[" + std::string(std::to_string(i)) + std::string("].LightSpaceMatrix")), lightSpaceMatrix);
				shader->setVec2(std::string("lights[" + std::string(std::to_string(i)) + std::string("].spotShadowAtlasTexOffset")), spotShadowAtlas->GetSlotStartXY(slotRow, slotColumn));
				shader->setVec2(std::string("lights[" + std::string(std::to_string(i)) + std::string("].shadowResolution")), glm::vec2(renderInstance->ShadowWidth(), renderInstance->ShadowHeight()));
			}
		}

		RenderOptions renderOptions = renderInstance->GetRenderParams()->GetRenderOptions();

		// Image based lighting
		shader->setBool("useGlobalIBL", false);
		shader->setInt("activeLocalIBLProbes", 0);
		if ((renderOptions & RENDER_IBL) != 0 && activeCamera != nullptr) {
			SetIBLUniforms(shader, activeCamera);
		}
	}
}