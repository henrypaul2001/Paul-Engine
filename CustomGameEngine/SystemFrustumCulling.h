#pragma once
#include "System.h"
#include "Camera.h"
#include "RenderManager.h"
#include <map>
namespace Engine {
	class SystemFrustumCulling : public System
	{
	public:
		SystemFrustumCulling(Camera* activeCamera) { this->activeCamera = activeCamera; this->viewFrustum = &activeCamera->GetViewFrustum(); }
		~SystemFrustumCulling();

		SystemTypes Name() override { return SYSTEM_FRUSTUM_CULLING; }
		void Run(const std::vector<Entity*>& entityList) override;
		void OnAction(Entity* entity) override;
		void AfterAction() override;

		void SetActiveCamera(Camera* newCamera) { this->activeCamera = newCamera; }
	private:
		const ComponentTypes GEOMETRY_MASK = (COMPONENT_TRANSFORM | COMPONENT_GEOMETRY);

		bool SphereIsOnOrInFrontOfPlane(const glm::vec3& spherePos, const float sphereRadius, const ViewPlane& plane);

		void CheckGeometry(ComponentTransform* transform, ComponentGeometry* geometry);
		void CullReflectionProbes();

		Camera* activeCamera;
		const ViewFrustum* viewFrustum;

		std::map<float, ReflectionProbe*> culledProbeList;
	};
}