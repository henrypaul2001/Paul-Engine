#include "SystemSkeletalAnimationUpdater.h"
#include "Scene.h"
namespace Engine {
	void SystemSkeletalAnimationUpdater::OnAction(const unsigned int entityID, ComponentGeometry& geometry, ComponentAnimator& animator)
	{
		AnimationSkeleton& skeleton = *geometry.GetModel()->GetAnimationSkeleton();

		if (skeleton.bones.size() > 0) { animator.UpdateAnimation(Scene::dt, skeleton); }
	}

	void SystemSkeletalAnimationUpdater::AfterAction() {}
}