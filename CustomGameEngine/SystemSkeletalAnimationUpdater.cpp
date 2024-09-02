#include "SystemSkeletalAnimationUpdater.h"
#include "Scene.h"
namespace Engine {
	SystemSkeletalAnimationUpdater::SystemSkeletalAnimationUpdater()
	{

	}

	SystemSkeletalAnimationUpdater::~SystemSkeletalAnimationUpdater()
	{

	}

	void SystemSkeletalAnimationUpdater::Run(const std::vector<Entity*>& entityList)
	{
		SCOPE_TIMER("SystemSkeletalAnimationUpdater::Run");
		System::Run(entityList);
	}

	void SystemSkeletalAnimationUpdater::OnAction(Entity* entity)
	{
		if ((entity->Mask() & MASK) == MASK) {
			std::vector<Component*> components = entity->Components();

			ComponentGeometry* geometry = nullptr;
			ComponentAnimator* animator = nullptr;
			
			for (Component* c : components) {
				if (c->ComponentType() == COMPONENT_GEOMETRY) {
					geometry = dynamic_cast<ComponentGeometry*>(c);
					continue;
				}
				else if (c->ComponentType() == COMPONENT_ANIMATOR) {
					animator = dynamic_cast<ComponentAnimator*>(c);
					continue;
				}

				if (geometry != nullptr && animator != nullptr) {
					UpdateAnimator(geometry, animator);
					return;
				}
			}

			if (geometry != nullptr && animator != nullptr) {
				UpdateAnimator(geometry, animator);
			}
		}
	}

	void SystemSkeletalAnimationUpdater::AfterAction()
	{

	}

	void SystemSkeletalAnimationUpdater::UpdateAnimator(ComponentGeometry* geometry, ComponentAnimator* animator)
	{
		AnimationSkeleton& skeleton = geometry->GetModel()->GetAnimationSkeleton();

		if (skeleton.bones.size() > 0) {
			animator->UpdateAnimation(Scene::dt, skeleton);
		}
	}
}