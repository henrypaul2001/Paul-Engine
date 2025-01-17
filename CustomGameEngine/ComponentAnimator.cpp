#include "ComponentAnimator.h"
namespace Engine {
	ComponentAnimator::ComponentAnimator(SkeletalAnimation* animation, const bool paused) : currentTime(0.0f), currentAnimation(animation), deltaTime(0.0f), speedModifier(1.0f), paused(paused) {}

	ComponentAnimator::~ComponentAnimator() {}

	void ComponentAnimator::UpdateAnimation(float deltaTime, AnimationSkeleton& animationTarget)
	{
		deltaTime *= speedModifier;
		if (!paused) {
			currentTime += currentAnimation->GetTicksPerSecond() * deltaTime;
			currentTime = fmod(currentTime, currentAnimation->GetDuration());
		}
		currentAnimation->Update(deltaTime, currentTime, animationTarget);
		finalBoneMatrices = animationTarget.finalBoneMatrices;
	}

	void ComponentAnimator::ChangeAnimation(SkeletalAnimation* newAnimation)
	{
		currentAnimation = newAnimation;
		currentTime = 0.0f;
	}
}