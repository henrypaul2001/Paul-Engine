#pragma once
#include "AnimationChannel.h"
#include "Model.h"
namespace Engine {
	class SkeletalAnimation
	{
	public:
		SkeletalAnimation(std::vector<AnimationChannel> animationChannels, float duration, float ticksPerSecond);
		~SkeletalAnimation();

		void Update(const float deltaTime, const float currentTime, AnimationSkeleton& animationTarget);

		const AnimationChannel* GetAnimationChannelAtIndex(const int index) const { return &channels[index]; }
		const AnimationChannel* GetAnimationChannelByName(const std::string& name);

		const float GetTicksPerSecond() const { return ticksPerSecond; }
		const float GetDuration() const { return duration; }

		void SetDuration(float newDuration) { this->duration = newDuration; }
		void SetTicksPerSecond(float newTPS) { this->ticksPerSecond = newTPS; }
	private:
		AnimationChannel* FindAnimationChannel(const std::string& name);
		void CalculateBoneTransformsRecursive(const AnimationBone& bone, const glm::mat4& parentTransform, AnimationSkeleton& animationTarget, const float currentTime);

		float duration;
		int ticksPerSecond;
		std::vector<AnimationChannel> channels;
	};
}