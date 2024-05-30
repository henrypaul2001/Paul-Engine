#pragma once
#include "Component.h"
#include "SkeletalAnimation.h"
namespace Engine {
	class ComponentAnimator : public Component
	{
	public:
		ComponentAnimator(SkeletalAnimation* animation, bool paused = false);
		~ComponentAnimator();

		ComponentTypes ComponentType() override { return COMPONENT_ANIMATOR; }
		void Close() override;

		const std::vector<glm::mat4>& GetFinalBonesMatrices() const { return finalBoneMatrices; }

		const bool Paused() const { return paused; }
		const float SpeedModifier() const { return speedModifier; }

		void UpdateAnimation(float deltaTime, AnimationSkeleton& animationTarget);
		void ChangeAnimation(SkeletalAnimation* newAnimation);
		void PauseAnimation() { paused = true; }
		void ResumeAnimation() { paused = false; }
		void SetPause(const bool isPaused) { paused = isPaused; }
		void SetSpeedModifier(float newSpeed) { speedModifier = newSpeed; }
	private:
		bool paused;
		SkeletalAnimation* currentAnimation;
		float currentTime;
		float deltaTime;
		float speedModifier;

		std::vector<glm::mat4> finalBoneMatrices;
	};
}