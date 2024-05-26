#include "AnimationChannel.h"
#include <glm/gtx/quaternion.hpp>

namespace Engine {
	AnimationChannel::AnimationChannel(const std::string& name, int ID, std::vector<AnimKeyPosition> positions, std::vector<AnimKeyRotation> rotations, std::vector<AnimKeyScale> scalings)
	{
		this->name = name;
		this->ID = ID;
		this->positions = positions;
		this->rotations = rotations;
		this->scalings = scalings;
		processedTransform = glm::mat4(1.0f);
	}

	AnimationChannel::~AnimationChannel() {}

	void AnimationChannel::Update(float animationTime)
	{
		glm::mat4 translation = InterpolatePosition(animationTime);
		glm::mat4 rotation = InterpolateRotation(animationTime);
		glm::mat4 scale = InterpolateScaling(animationTime);
		processedTransform = translation * rotation * scale;
	}

	int AnimationChannel::GetPositionIndex(float animationTime)
	{
		for (int index = 0; index < positions.size(); ++index) {
			if (animationTime < positions[index + 1].timeStamp) { return index; }
		}
		return -1;
	}

	int AnimationChannel::GetRotationIndex(float animationTime)
	{
		for (int index = 0; index < rotations.size(); ++index) {
			if (animationTime < rotations[index + 1].timeStamp) { return index; }
		}
		return -1;
	}

	int AnimationChannel::GetScaleIndex(float animationTime)
	{
		for (int index = 0; index < scalings.size(); ++index) {
			if (animationTime < scalings[index + 1].timeStamp) { return index; }
		}
		return -1;
	}

	float AnimationChannel::GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime)
	{
		float scaleFactor = 0.0f;
		float midwayLength = animationTime - lastTimeStamp;
		float framesDiff = nextTimeStamp - lastTimeStamp;
		scaleFactor = midwayLength / framesDiff;
		return scaleFactor;
	}

	glm::mat4 AnimationChannel::InterpolatePosition(float animationTime)
	{
		if (positions.size() > 0) {
			if (positions.size() == 1) {
				return glm::translate(glm::mat4(1.0f), positions[0].position);
			}

			int p0Index = GetPositionIndex(animationTime);
			int p1Index = p0Index + 1;

			if (p0Index != -1) {
				float scaleFactor = GetScaleFactor(positions[p0Index].timeStamp, positions[p1Index].timeStamp, animationTime);
				glm::vec3 finalPosition = glm::mix(positions[p0Index].position, positions[p1Index].position, scaleFactor);

				return glm::translate(glm::mat4(1.0f), finalPosition);
			}
		}
		return glm::mat4(1.0f);
	}

	glm::mat4 AnimationChannel::InterpolateRotation(float animationTime)
	{
		if (rotations.size() > 0) {
			if (rotations.size() == 1) {
				glm::quat rotation = glm::normalize(rotations[0].orientation);
				return glm::toMat4(rotation);
			}

			int p0Index = GetRotationIndex(animationTime);
			int p1Index = p0Index + 1;

			if (p0Index != -1) {
				float scaleFactor = GetScaleFactor(rotations[p0Index].timeStamp, rotations[p1Index].timeStamp, animationTime);
				glm::quat finalRotation = glm::slerp(rotations[p0Index].orientation, rotations[p1Index].orientation, scaleFactor);

				return glm::toMat4(finalRotation);
			}
		}
		return glm::mat4(1.0f);
	}

	glm::mat4 AnimationChannel::InterpolateScaling(float animationTime)
	{
		if (scalings.size() > 0) {
			if (scalings.size() == 1) {

			}

			int p0Index = GetScaleIndex(animationTime);
			int p1Index = p0Index + 1;

			if (p0Index != -1) {
				float scaleFactor = GetScaleFactor(scalings[p0Index].timeStamp, scalings[p1Index].timeStamp, animationTime);
				glm::vec3 finalScale = glm::mix(scalings[p0Index].scale, scalings[p1Index].scale, scaleFactor);
				
				return glm::scale(glm::mat4(1.0f), finalScale);
			}
		}
		return glm::mat4(1.0f);
	}
}