#include "SystemFrustumCulling.h"
namespace Engine {
	SystemFrustumCulling::~SystemFrustumCulling()
	{

	}

	void SystemFrustumCulling::Run(const std::vector<Entity*>& entityList)
	{
		viewFrustum = &activeCamera->GetViewFrustum();

		System::Run(entityList);
	
		CullReflectionProbes();
	}

	void SystemFrustumCulling::OnAction(Entity* entity)
	{
		if ((entity->Mask() & GEOMETRY_MASK) == GEOMETRY_MASK) {
			std::vector<Component*> components = entity->Components();

			ComponentTransform* transform = nullptr;
			for (Component* c : components) {
				transform = dynamic_cast<ComponentTransform*>(c);
				if (transform != nullptr) {
					break;
				}
			}

			ComponentGeometry* geometry = nullptr;
			for (Component* c : components) {
				geometry = dynamic_cast<ComponentGeometry*>(c);
				if (geometry != nullptr) {
					break;
				}
			}

			CheckGeometry(transform, geometry);
		}
	}

	void SystemFrustumCulling::AfterAction()
	{

	}

	bool SystemFrustumCulling::SphereIsOnOrInFrontOfPlane(const glm::vec3& spherePos, const float sphereRadius, const ViewPlane& plane)
	{
		float distanceToPlane = glm::dot(plane.normal, spherePos) - plane.distance;

		return distanceToPlane >= -sphereRadius;
	}

	void SystemFrustumCulling::CheckGeometry(ComponentTransform* transform, ComponentGeometry* geometry)
	{
		// not yet implemented
	}

	void SystemFrustumCulling::CullReflectionProbes()
	{
		RenderManager* renderManager = RenderManager::GetInstance();
		std::vector<ReflectionProbe*> reflectionProbes = renderManager->GetBakedData().GetReflectionProbes();
		unsigned int culledProbes = 0;
		culledProbeList.clear();

		for (ReflectionProbe* probe : reflectionProbes) {
			float soiRadius = probe->GetSOIRadius();
			glm::vec3 worldPos = probe->GetWorldPosition();

			// Check for collision on each view plane, starting with most likely to fail first
			if (SphereIsOnOrInFrontOfPlane(worldPos, soiRadius, viewFrustum->left) && SphereIsOnOrInFrontOfPlane(worldPos, soiRadius, viewFrustum->right) &&
				SphereIsOnOrInFrontOfPlane(worldPos, soiRadius, viewFrustum->far) && SphereIsOnOrInFrontOfPlane(worldPos, soiRadius, viewFrustum->near) &&
				SphereIsOnOrInFrontOfPlane(worldPos, soiRadius, viewFrustum->top) && SphereIsOnOrInFrontOfPlane(worldPos, soiRadius, viewFrustum->bottom))
			{
				// Probe is inside view frustum
				float distanceToCameraSquared = glm::distance2(activeCamera->GetPosition(), worldPos);
				if (culledProbeList.find(distanceToCameraSquared) != culledProbeList.end()) {
					// Distance already exists, increment slightly
					distanceToCameraSquared += 0.00001f;
				}
				culledProbeList[distanceToCameraSquared] = probe;
				culledProbes++;
			}
		}

		renderManager->GetBakedData().SetCulledProbeList(culledProbeList);

		std::cout << "Reflection probes in view frustum: " << culledProbes << std::endl;
	}
}