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

		const std::vector<glm::mat4>& GetFinalBoneMatrices() const { return finalBoneMatrices; }

		const float GetTicksPerSecond() const { return ticksPerSecond; }
		const float GetDuration() const { return duration; }

	private:
		AnimationChannel* FindAnimationChannel(const std::string& name);
		void CalculateBoneTransformsRecursive(const AnimationBone& bone, const glm::mat4& parentTransform, const AnimationSkeleton& animationTarget, const float currentTime);

		float duration;
		int ticksPerSecond;
		std::vector<AnimationChannel> channels;
		std::vector<glm::mat4> finalBoneMatrices;
	};
}