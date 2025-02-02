#include "SystemSkeletalAnimationUpdater.h"
#include "Scene.h"
namespace Engine {
	void SystemSkeletalAnimationUpdater::OnAction(const unsigned int entityID, ComponentGeometry& geometry, ComponentAnimator& animator)
	{
		SCOPE_TIMER("SystemSkeletalAnimationUpdater::OnAction()");
		AnimationSkeleton* skeleton = geometry.GetModel()->GetAnimationSkeleton();

		if (skeleton && skeleton->bones.size() > 0) { animator.UpdateAnimation(Scene::dt, *skeleton); }
	}

	void SystemSkeletalAnimationUpdater::AfterAction() {}
}