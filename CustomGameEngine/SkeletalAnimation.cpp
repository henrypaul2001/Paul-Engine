#include "SkeletalAnimation.h"
#include <map>
namespace Engine {
	SkeletalAnimation::SkeletalAnimation(std::vector<AnimationChannel> animationChannels, float duration, float ticksPerSecond)
	{
		this->channels = animationChannels;
		this->duration = duration;
		this->ticksPerSecond = ticksPerSecond;
	}

	SkeletalAnimation::~SkeletalAnimation() {}

	void SkeletalAnimation::Update(const float deltaTime, const float currentTime, AnimationSkeleton& animationTarget)
	{
		CalculateBoneTransformsRecursive(*animationTarget.rootBone, glm::mat4(1.0f), animationTarget, currentTime);
	}

	const AnimationChannel* SkeletalAnimation::GetAnimationChannelByName(const std::string& name)
	{
		for (AnimationChannel& channel : channels) {
			if (channel.GetChannelName() == name) { return &channel; }
		}
		return nullptr;
	}

	AnimationChannel* SkeletalAnimation::FindAnimationChannel(const std::string& name)
	{
		for (AnimationChannel& channel : channels) {
			if (channel.GetChannelName() == name) { return &channel; }
		}
		return nullptr;
	}

	void SkeletalAnimation::CalculateBoneTransformsRecursive(const AnimationBone& bone, const glm::mat4& parentTransform, AnimationSkeleton& animationTarget, const float currentTime)
	{
		std::string boneName = bone.name;
		glm::mat4 nodeTransform = bone.nodeTransform;

		AnimationChannel* channel = FindAnimationChannel(boneName);

		if (channel) {
			channel->Update(currentTime);
			nodeTransform = channel->GetProcessedTransform();
		}

		glm::mat4 globalTransformation = parentTransform * nodeTransform;

		int index = bone.boneID;
		glm::mat4 offset = glm::mat4(1.0f);

		if (index > -1) {
			offset = bone.offsetMatrix;

			animationTarget.finalBoneMatrices[index] = globalTransformation * offset;
		}


		std::map<std::string, AnimationBone> bones = animationTarget.bones;
		std::map<std::string, AnimationBone> emptyBones = animationTarget.emptyBones;

		for (const std::string& childBoneName : bone.childNodeNames) {
			AnimationBone* child = nullptr;

			if (bones.find(childBoneName) != bones.end()) {
				child = &bones[childBoneName];
			}
			else if (emptyBones.find(childBoneName) != emptyBones.end()) {
				child = &emptyBones[childBoneName];
			}

			if (child) {
				CalculateBoneTransformsRecursive(*child, globalTransformation, animationTarget, currentTime);
			}
		}
	}
}