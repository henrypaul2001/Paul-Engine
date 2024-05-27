#pragma once
#include "Component.h"
#include "SkeletalAnimation.h"
namespace Engine {
	class ComponentAnimator : public Component
	{
	public:
		ComponentAnimator(SkeletalAnimation* animation, bool paused = false);
		~ComponentAnimator();

		const bool Paused() const { return paused; }

		void UpdateAnimation(float deltaTime, AnimationSkeleton& animationTarget);
		void ChangeAnimation(SkeletalAnimation* newAnimation);
		void PauseAnimation() { paused = true; }
		void ResumeAnimation() { paused = false; }
		void SetPause(const bool isPaused) { paused = isPaused; }
	private:
		bool paused;
		SkeletalAnimation* currentAnimation;
		float currentTime;
		float deltaTime;
	};
}