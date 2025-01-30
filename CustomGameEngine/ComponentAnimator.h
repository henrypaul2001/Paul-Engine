#pragma once
#include "SkeletalAnimation.h"
namespace Engine {
	class ComponentAnimator
	{
	public:
		ComponentAnimator(SkeletalAnimation* animation, const bool paused = false);
		~ComponentAnimator();

		const std::vector<glm::mat4>& GetFinalBonesMatrices() const { return finalBoneMatrices; }

		const bool Paused() const { return paused; }
		const float SpeedModifier() const { return speedModifier; }

		void UpdateAnimation(const float deltaTime, AnimationSkeleton& animationTarget);
		void ChangeAnimation(SkeletalAnimation* newAnimation);
		void PauseAnimation() { paused = true; }
		void ResumeAnimation() { paused = false; }
		void SetPause(const bool isPaused) { paused = isPaused; }
		void SetSpeedModifier(const float newSpeed) { speedModifier = newSpeed; }

	private:
		bool paused;
		float currentTime;
		float deltaTime;
		float speedModifier;

		std::vector<glm::mat4> finalBoneMatrices;
		SkeletalAnimation* currentAnimation;
	};
}