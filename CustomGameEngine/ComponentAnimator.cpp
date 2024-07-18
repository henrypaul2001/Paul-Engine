#include "ComponentAnimator.h"
namespace Engine {
	ComponentAnimator::ComponentAnimator(const ComponentAnimator& old_component)
	{
		this->owner = nullptr;

		this->currentTime = old_component.currentTime;
		this->currentAnimation = old_component.currentAnimation;
		this->paused = old_component.paused;
		this->deltaTime = old_component.deltaTime;
		this->speedModifier = old_component.speedModifier;
		this->finalBoneMatrices = old_component.finalBoneMatrices;
	}

	ComponentAnimator::ComponentAnimator(SkeletalAnimation* animation, bool paused)
	{
		currentTime = 0.0f;
		currentAnimation = animation;
		deltaTime = 0.0f;
		speedModifier = 1.0f;
		this->paused = paused;
	}

	ComponentAnimator::~ComponentAnimator()
	{

	}

	void ComponentAnimator::Close()
	{

	}

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