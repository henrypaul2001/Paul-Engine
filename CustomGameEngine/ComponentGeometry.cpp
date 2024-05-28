#include "ComponentGeometry.h"
#include "SceneManager.h"
namespace Engine {
	ComponentGeometry::ComponentGeometry(PremadeModel modelType, const char* vShaderFilepath, const char* fShaderFilepath, bool pbr, bool instanced)
	{
		this->instanced = instanced;
		this->pbr = pbr;
		model = new Model(modelType, pbr);

		CULL_FACE = true;
		CULL_TYPE = GL_BACK;

		if (modelType == MODEL_PLANE) {
			//CULL_FACE = false;
			CULL_TYPE = GL_BACK;
		}

		usingDefaultShader = false;

		castShadows = true;

		textureScale = 1.0f;

		shader = ResourceManager::GetInstance()->LoadShader(vShaderFilepath, fShaderFilepath);

		if (instanced) { SetupInstanceVBO(); }
	}

	ComponentGeometry::ComponentGeometry(PremadeModel modelType, bool pbr, bool instanced)
	{
		this->instanced = instanced;
		this->pbr = pbr;
		model = new Model(modelType, pbr);

		CULL_FACE = true;
		CULL_TYPE = GL_BACK;

		if (modelType == MODEL_PLANE) {
			//CULL_FACE = false;
			CULL_TYPE = GL_BACK;
		}

		usingDefaultShader = true;

		castShadows = true;

		shader = nullptr;
		if (RenderManager::GetInstance()->GetRenderPipeline()->Name() == FORWARD_PIPELINE) {
			if (pbr) {
				shader = ResourceManager::GetInstance()->DefaultLitPBR();
			}
			else {
				shader = ResourceManager::GetInstance()->DefaultLitShader();
			}
		}
		else if (RenderManager::GetInstance()->GetRenderPipeline()->Name() == DEFERRED_PIPELINE) {
			if (pbr) {
				shader = ResourceManager::GetInstance()->DeferredGeometryPassPBR();
			}
			else {
				shader = ResourceManager::GetInstance()->DeferredGeometryPass();
			}
		}

		textureScale = 1.0f;

		if (instanced) { SetupInstanceVBO(); }
	}

	ComponentGeometry::ComponentGeometry(const char* modelFilepath, const char* vShaderFilepath, const char* fShaderFilepath, bool pbr, bool instanced)
	{
		this->instanced = instanced;
		this->pbr = pbr;

		model = ResourceManager::GetInstance()->LoadModel(modelFilepath, pbr);

		usingDefaultShader = false;

		castShadows = true;

		textureScale = 1.0f;

		CULL_FACE = true;
		CULL_TYPE = GL_BACK;

		shader = ResourceManager::GetInstance()->LoadShader(vShaderFilepath, fShaderFilepath);

		if (instanced) { SetupInstanceVBO(); }
	}

	ComponentGeometry::ComponentGeometry(const char* modelFilepath, bool pbr, bool instanced)
	{
		this->instanced = instanced;
		CULL_FACE = true;
		CULL_TYPE = GL_BACK;

		this->pbr = pbr;

		usingDefaultShader = true;

		castShadows = true;

		model = ResourceManager::GetInstance()->LoadModel(modelFilepath, pbr);

		shader = nullptr;
		if (RenderManager::GetInstance()->GetRenderPipeline()->Name() == FORWARD_PIPELINE) {
			if (pbr) {
				shader = ResourceManager::GetInstance()->DefaultLitPBR();
			}
			else {
				shader = ResourceManager::GetInstance()->DefaultLitShader();
			}
		}
		else if (RenderManager::GetInstance()->GetRenderPipeline()->Name() == DEFERRED_PIPELINE) {
			if (pbr) {
				shader = ResourceManager::GetInstance()->DeferredGeometryPassPBR();
			}
			else {
				shader = ResourceManager::GetInstance()->DeferredGeometryPass();
			}
		}

		textureScale = 1.0f;

		if (instanced) { SetupInstanceVBO(); }
	}

	void ComponentGeometry::RemoveInstanceSource(Entity* sourceToRemove)
	{
		for (int i = 0; i < instanceSources.size(); i++) {
			if (instanceSources[i] == sourceToRemove) {
				instanceSources.erase(instanceSources.begin() + i);
				instanceTransforms.erase(instanceTransforms.begin() + i);
				return;
			}
		}
	}

	void ComponentGeometry::BufferInstanceTransforms()
	{
		//const std::vector<Entity*>& sources = InstanceSources();
		ResizeInstancedTransforms();

		//geometry->UpdateInstanceTransform(0, transform->GetWorldModelMatrix());
		for (int i = 0; i < instanceSources.size(); i++) {
			UpdateInstanceTransform(i, instanceSources[i]->GetTransformComponent()->GetWorldModelMatrix());
		}

		glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
		glBufferData(GL_ARRAY_BUFFER, instanceSources.size() * sizeof(glm::mat4), &instanceTransforms[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void ComponentGeometry::OnAddedToEntity()
	{
		if (instanced) {
			AddNewInstanceSource(GetOwner());
		}
	}

	void ComponentGeometry::SetupInstanceVBO()
	{
		glGenBuffers(1, &instanceVBO);
		glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);

		for (int i = 0; i < model->meshes.size(); i++) {
			unsigned int VAO = model->meshes[i]->VAO;
			glBindVertexArray(VAO);

			glEnableVertexAttribArray(7);
			glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
			glEnableVertexAttribArray(8);
			glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
			glEnableVertexAttribArray(9);
			glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
			glEnableVertexAttribArray(10);
			glVertexAttribPointer(10, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

			glVertexAttribDivisor(7, 1);
			glVertexAttribDivisor(8, 1);
			glVertexAttribDivisor(9, 1);
			glVertexAttribDivisor(10, 1);

			glBindVertexArray(0);
		}

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	ComponentGeometry::~ComponentGeometry()
	{

	}

	void ComponentGeometry::Close()
	{

	}
}