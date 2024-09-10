#include "Model.h"
#include "ResourceManager.h"
#include "ComponentGeometry.h"
namespace Engine {
	Model::Model(const Model& old_model)
	{
		for (Mesh* oldMesh : old_model.meshes) {
			this->meshes.push_back(new Mesh(*oldMesh));
		}
		this->pbr = old_model.pbr;
		this->hasBones = old_model.hasBones;
		this->containsTransparentMeshes = old_model.containsTransparentMeshes;
		this->directory = old_model.directory;
		this->skeleton = old_model.skeleton;
		this->hasBones = old_model.hasBones;

		for (unsigned int i = 0; i < meshes.size(); i++) {
			Mesh* m = meshes[i];
			m->SetOwner(this);
			m->SetLocalMeshID(i);
		}
	}

	Model::Model(const std::vector<Mesh*>& meshes, bool pbr)
	{
		this->meshes = meshes;
		this->hasBones = false;
		this->pbr = pbr;

		for (unsigned int i = 0; i < meshes.size(); i++) {
			Mesh* m = meshes[i];
			m->SetOwner(this);
			m->SetLocalMeshID(i);
		}
	}

	Model::Model(PremadeModel modelType, bool pbr)
	{
		if (modelType == MODEL_PLANE) {
			meshes.push_back(new Mesh(ResourceManager::GetInstance()->DefaultPlane()));
		}
		else if (modelType == MODEL_CUBE) {
			meshes.push_back(new Mesh(ResourceManager::GetInstance()->DefaultCube()));
		}
		else if (modelType == MODEL_SPHERE) {
			meshes.push_back(new Mesh(ResourceManager::GetInstance()->DefaultSphere()));
		}
		containsTransparentMeshes = false;
		hasBones = false;
		this->pbr = pbr;

		for (unsigned int i = 0; i < meshes.size(); i++) {
			Mesh* m = meshes[i];
			m->SetOwner(this);
			m->SetLocalMeshID(i);
		}
	}

	Model::~Model()
	{
		for (Mesh* m : meshes) {
			delete m;
		}
	}

	void Model::Draw(Shader& shader, int instanceNum, const std::vector<unsigned int> instanceVAOs)
	{
		SCOPE_TIMER("Model::Draw");
		for (unsigned int i = 0; i < meshes.size(); i++) {
			if (instanceNum > 0) {
				meshes[i]->Draw(shader, pbr, instanceNum, instanceVAOs[i]);
			}
			else {
				meshes[i]->Draw(shader, pbr, instanceNum);
			}
		}
	}

	void Model::DrawTransparentMeshes(Shader& shader, int instanceNum, const std::vector<unsigned int> instanceVAOs)
	{
		SCOPE_TIMER("Model::DrawTransparentMeshes");
		for (unsigned int i = 0; i < meshes.size(); i++) {
			if (meshes[i]->GetMaterial()->GetIsTransparent()) {
				if (instanceNum > 0) {
					meshes[i]->Draw(shader, pbr, instanceNum, instanceVAOs[i]);
				}
				else {
					meshes[i]->Draw(shader, pbr, instanceNum);
				}
			}
		}
	}

	void Model::ApplyMaterialsToAllMesh(const std::vector<AbstractMaterial*>& materials)
	{
		pbr = materials[0]->IsPBR();
		for (AbstractMaterial* m : materials) {
			if (m->GetIsTransparent()) {
				containsTransparentMeshes = true;
			}
		}

		for (Mesh* m : meshes) {
			m->SetMaterials(materials);
		}
	}

	void Model::ApplyMaterialsToMeshAtIndex(const std::vector<AbstractMaterial*>& materials, const unsigned int index)
	{
		pbr = materials[0]->IsPBR();
		for (AbstractMaterial* m : materials) {
			if (m->GetIsTransparent()) {
				containsTransparentMeshes = true;
			}
		}

		if (index < meshes.size()) {
			meshes[index]->SetMaterials(materials);
		}
	}
}