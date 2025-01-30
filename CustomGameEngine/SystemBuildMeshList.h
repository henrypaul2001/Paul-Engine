#pragma once
#include "System.h"
#include "ComponentTransform.h"
#include "ComponentGeometry.h"
#include "ComponentAnimator.h"

namespace Engine {
	class SystemBuildMeshList : public System {
	public:
		SystemBuildMeshList(EntityManagerNew* ecs) : System(ecs) {}
		~SystemBuildMeshList() {}

		constexpr const char* SystemName() override { return "SYSTEM_BUILD_MESH_LIST"; }

		void PreAction() {
			centrePosAndMeshesList.clear();
			centrePosAndMeshesList.reserve(active_ecs->NumEntities());
		}

		void OnAction(const unsigned int entityID, ComponentTransform& transform, ComponentGeometry& geometry) {
			SCOPE_TIMER("SystemBuildMeshList::OnAction");

			// Update geometry bounds for model
			if (!active_ecs->HasComponent<ComponentAnimator>(entityID)) { // animated meshes have their boundaries calculated by another system
				geometry.GetModel()->UpdateGeometryBoundingBoxes(transform.GetWorldModelMatrix());
			}

			const glm::vec3& pos = transform.GetWorldPosition(); // temporarily use entity pos as AABB centre

			const std::vector<Mesh*>& meshList = geometry.GetModel()->meshes;
			for (Mesh* m : meshList) {
				centrePosAndMeshesList.push_back(std::make_pair(std::make_pair(pos, entityID), m));
			}
		}

		static const std::vector<std::pair<std::pair<glm::vec3, unsigned int>, Mesh*>>& MeshList() { return centrePosAndMeshesList; }
	private:
		static std::vector<std::pair<std::pair<glm::vec3, unsigned int>, Mesh*>> centrePosAndMeshesList;
	};
}