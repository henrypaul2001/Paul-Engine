#include "ComponentGeometry.h"
#include "SceneManager.h"
namespace Engine {
	ComponentGeometry::ComponentGeometry(const ComponentGeometry& old_component)
	{
		this->usingPremadeModel = old_component.usingPremadeModel;

		this->model = new Model(*old_component.model);
		this->model->SetOwner(this);

		includeInReflectionProbes = true;

		this->shader = old_component.shader;

		this->textureScale = old_component.textureScale;
		this->castShadows = old_component.castShadows;

		this->pbr = old_component.pbr;
		this->usingDefaultShader = old_component.usingDefaultShader;

		this->CULL_TYPE = old_component.CULL_TYPE;
		this->CULL_FACE = old_component.CULL_FACE;

		//this->instanced = old_component.instanced;

		//if (instanced) { SetupInstanceVBO(); }

		//this->instanceTransforms = old_component.instanceTransforms;
		//this->instanceSources = old_component.instanceSources; // This should probably be cloned entities instead of pointing to the same original entities
	}

	ComponentGeometry::ComponentGeometry(PremadeModel modelType, const char* vShaderFilepath, const char* fShaderFilepath, bool pbr, bool instanced)
	{
		//this->instanced = instanced;
		this->pbr = pbr;
		model = new Model(modelType, pbr);
		usingPremadeModel = true;

		CULL_FACE = true;
		CULL_TYPE = GL_BACK;

		includeInReflectionProbes = true;

		if (modelType == MODEL_PLANE) {
			//CULL_FACE = false;
			CULL_TYPE = GL_BACK;
		}

		usingDefaultShader = false;

		castShadows = true;

		textureScale = glm::vec2(1.0f);

		shader = ResourceManager::GetInstance()->LoadShader(vShaderFilepath, fShaderFilepath);

		//if (instanced) { SetupInstanceVBO(); }

		if (pbr) { ApplyMaterialToModel(ResourceManager::GetInstance()->DefaultMaterialPBR()); }
		this->model->SetOwner(this);
	}

	ComponentGeometry::ComponentGeometry(PremadeModel modelType, bool pbr, bool instanced)
	{
		//this->instanced = instanced;
		this->pbr = pbr;
		model = new Model(modelType, pbr);
		usingPremadeModel = true;

		CULL_FACE = true;
		CULL_TYPE = GL_BACK;

		includeInReflectionProbes = true;

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

		textureScale = glm::vec2(1.0f);

		//if (instanced) { SetupInstanceVBO(); }

		if (pbr) { ApplyMaterialToModel(ResourceManager::GetInstance()->DefaultMaterialPBR()); }
		this->model->SetOwner(this);
	}

	ComponentGeometry::ComponentGeometry(const char* modelFilepath, const char* vShaderFilepath, const char* fShaderFilepath, bool pbr, bool instanced, bool persistentStorage, const unsigned int assimpPostProcess)
	{
		//this->instanced = instanced;
		this->pbr = pbr;

		includeInReflectionProbes = true;

		model = ResourceManager::GetInstance()->CreateModel(modelFilepath, pbr, persistentStorage, assimpPostProcess);
		usingPremadeModel = false;

		usingDefaultShader = false;

		castShadows = true;

		textureScale = glm::vec2(1.0f);

		CULL_FACE = true;
		CULL_TYPE = GL_BACK;

		shader = ResourceManager::GetInstance()->LoadShader(vShaderFilepath, fShaderFilepath);

		//if (instanced) { SetupInstanceVBO(); }
		this->model->SetOwner(this);
	}

	ComponentGeometry::ComponentGeometry(const char* modelFilepath, bool pbr, bool instanced, bool persistentStorage, const unsigned int assimpPostProcess)
	{
		//this->instanced = instanced;
		CULL_FACE = true;
		CULL_TYPE = GL_BACK;

		includeInReflectionProbes = true;

		this->pbr = pbr;

		usingDefaultShader = true;

		castShadows = true;

		model = ResourceManager::GetInstance()->CreateModel(modelFilepath, pbr, persistentStorage, assimpPostProcess);
		usingPremadeModel = false;

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

		textureScale = glm::vec2(1.0f);

		//if (instanced) { SetupInstanceVBO(); }
		this->model->SetOwner(this);
	}

	//void ComponentGeometry::RemoveInstanceSource(Entity* sourceToRemove)
	//{
	//	for (int i = 0; i < instanceSources.size(); i++) {
	//		if (instanceSources[i] == sourceToRemove) {
	//			instanceSources.erase(instanceSources.begin() + i);
	//			instanceTransforms.erase(instanceTransforms.begin() + i);
	//			return;
	//		}
	//	}
	//}

	//void ComponentGeometry::BufferInstanceTransforms()
	//{
	//	//const std::vector<Entity*>& sources = InstanceSources();
	//	ResizeInstancedTransforms();

	//	//geometry->UpdateInstanceTransform(0, transform->GetWorldModelMatrix());
	//	for (int i = 0; i < instanceSources.size(); i++) {
	//		UpdateInstanceTransform(i, instanceSources[i]->GetTransformComponent()->GetWorldModelMatrix());
	//	}

	//	for (unsigned int i = 0; i < instanceVBOs.size(); i++) {
	//		glBindBuffer(GL_ARRAY_BUFFER, instanceVBOs[i]);
	//		glBufferData(GL_ARRAY_BUFFER, instanceSources.size() * sizeof(glm::mat4), &instanceTransforms[0], GL_STATIC_DRAW);
	//		glBindBuffer(GL_ARRAY_BUFFER, 0);
	//	}
	//}

	//void ComponentGeometry::OnAddedToEntity()
	//{
	//	if (instanced) {
	//		AddNewInstanceSource(GetOwner());
	//	}

	//	model->UpdateGeometryBoundingBoxes(owner->GetTransformComponent()->GetWorldModelMatrix());
	//}

	void ComponentGeometry::ApplyMaterialSetToModel(const std::vector<AbstractMaterial*>& newMaterials)
	{
		bool newPBR = newMaterials[0]->IsPBR();
		bool changedMatType = (pbr != newPBR);
		pbr = newPBR;
		if (changedMatType && usingDefaultShader) {
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
		}

		model->ApplyMaterialsToAllMesh(newMaterials);
	}

	//void ComponentGeometry::SetupInstanceVBO()
	//{
	//	instanceVAOs.resize(model->meshes.size());
	//	instanceVBOs.resize(model->meshes.size());

	//	for (int i = 0; i < model->meshes.size(); i++) {
	//		glGenBuffers(1, &instanceVBOs[i]);
	//		glBindBuffer(GL_ARRAY_BUFFER, instanceVBOs[i]);

	//		const MeshData& meshData = model->meshes[i]->GetMeshData();
	//		const unsigned int meshVAO = meshData.GetVAO();
	//		const unsigned int meshVBO = meshData.GetVBO();
	//		const unsigned int meshEBO = meshData.GetEBO();

	//		// Generate new VAO for instanced rendering
	//		glGenVertexArrays(1, &instanceVAOs[i]);
	//		glBindVertexArray(instanceVAOs[i]);

	//		// Bind mesh VBO and set up vertex attributes
	//		glBindBuffer(GL_ARRAY_BUFFER, meshVBO);
	//		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshEBO);

	//		// base
	//		{
	//			glEnableVertexAttribArray(0);
	//			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

	//			// normals
	//			glEnableVertexAttribArray(1);
	//			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

	//			// texture coords
	//			glEnableVertexAttribArray(2);
	//			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

	//			// vertex tangent
	//			glEnableVertexAttribArray(3);
	//			glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));

	//			// vertex bitangent
	//			glEnableVertexAttribArray(4);
	//			glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

	//			// ids
	//			glEnableVertexAttribArray(5);
	//			glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, BoneIDs));

	//			// weights
	//			glEnableVertexAttribArray(6);
	//			glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, BoneWeights));
	//		}

	//		// instancing
	//		{
	//			glBindBuffer(GL_ARRAY_BUFFER, instanceVBOs[i]);
	//			glEnableVertexAttribArray(7);
	//			glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
	//			glEnableVertexAttribArray(8);
	//			glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
	//			glEnableVertexAttribArray(9);
	//			glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
	//			glEnableVertexAttribArray(10);
	//			glVertexAttribPointer(10, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

	//			glVertexAttribDivisor(7, 1);
	//			glVertexAttribDivisor(8, 1);
	//			glVertexAttribDivisor(9, 1);
	//			glVertexAttribDivisor(10, 1);

	//			glBindVertexArray(0);
	//		}
	//	}
	//	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//}

	ComponentGeometry::~ComponentGeometry()
	{
		if (model) { delete model; }
	}
}