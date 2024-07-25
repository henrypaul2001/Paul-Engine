#pragma once
#include "Component.h"
#include "Model.h"
//#include "Shader.h"
namespace Engine {
	class ComponentGeometry : public Component
	{
	public:
		ComponentGeometry(const ComponentGeometry& old_component);
		ComponentGeometry(PremadeModel modelType, const char* vShaderFilepath, const char* fShaderFilepath, bool pbr, bool instanced = false);
		ComponentGeometry(PremadeModel modelType, bool pbr = false, bool instanced = false);
		ComponentGeometry(const char* modelFilepath, const char* vShaderFilepath, const char* fShaderFilepath, bool pbr, bool instanced = false, bool persistentStorage = false, const unsigned int assimpPostProcess = defaultAssimpPostProcess);
		ComponentGeometry(const char* modelFilepath, bool pbr, bool instanced = false, bool persistentStorage = false, const unsigned int assimpPostProcess = defaultAssimpPostProcess);
		~ComponentGeometry();

		ComponentTypes ComponentType() override { return COMPONENT_GEOMETRY; }
		void Close() override;

		Component* Copy() override { return new ComponentGeometry(*this); }

		bool PBR() { return pbr; }
		void PBR(bool PBR) { 
			pbr = PBR;
			if (model != nullptr) {
				model->PBR(PBR);
			}
		}

		void SetCulling(bool cull, GLenum cull_type) { CULL_FACE = cull, CULL_TYPE = cull_type; }
		void SetShader(Shader* newShader) { shader = newShader; }

		bool Cull_Face() { return CULL_FACE; }
		GLenum Cull_Type() { return CULL_TYPE; }

		Model* GetModel() { return model; }
		Shader* GetShader() { return shader; }

		glm::vec2 GetTextureScale() { return textureScale; }
		
		void CastShadows(bool shadows) { castShadows = shadows; }
		bool CastShadows() { return castShadows; }

		void SetTextureScale(float newScale) { textureScale = glm::vec2(newScale); }
		void SetTextureScale(glm::vec2 newScale) { textureScale = newScale; }

		bool Instanced() { return instanced; }
		const int NumInstances() { return instanceSources.size(); }
		const std::vector<Entity*>& InstanceSources() { return instanceSources; }
		const std::vector<glm::mat4>& InstanceTransforms() { return instanceTransforms; }
		unsigned int InstanceVBO() { return instanceVBO; }

		void AddNewInstanceSource(Entity* newSource) { instanceSources.push_back(newSource); }
		void RemoveInstanceSource(Entity* sourceToRemove);
		void UpdateInstanceTransform(int index, glm::mat4 transform) { instanceTransforms[index] = transform; }
		void ResizeInstancedTransforms() { 
			if (instanceTransforms.size() != instanceSources.size()) {
				instanceTransforms.resize(instanceSources.size());
			}
		}

		void BufferInstanceTransforms();

		void OnAddedToEntity() override;

		void ApplyMaterialToModel(Material* newMaterial);
		void ApplyMaterialToModel(PBRMaterial* newMaterial);

		const bool IsIncludedInReflectionProbes() const { return includeInReflectionProbes; }
		void SetIsIncludedInReflectionProbes(const bool included) { includeInReflectionProbes = included; }
	private:
		Model* model;
		Shader* shader;

		bool usingPremadeModel;

		glm::vec2 textureScale;
		bool castShadows;

		bool pbr;
		bool usingDefaultShader;

		GLenum CULL_TYPE;
		bool CULL_FACE;

		unsigned int instanceVBO;
		bool instanced;
		std::vector<glm::mat4> instanceTransforms;
		std::vector<Entity*> instanceSources;
		
		bool includeInReflectionProbes;

		void SetupInstanceVBO();
	};
}