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

			ComponentGeometry* geometry = entity->GetGeometryComponent();
			ComponentAnimator* animator = entity->GetAnimator();
			
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
		AnimationSkeleton& skeleton = *geometry->GetModel()->GetAnimationSkeleton();

		if (skeleton.bones.size() > 0) {
			animator->UpdateAnimation(Scene::dt, skeleton);
		}
	}
}