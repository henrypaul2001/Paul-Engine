#pragma once
#include <string>
#include <vector>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/quaternion_float.hpp>
namespace Engine {

	struct AnimKeyFrame {
		float timeStamp;
	};

	struct AnimKeyPosition : public AnimKeyFrame {
		glm::vec3 position;
	};

	struct AnimKeyRotation : public AnimKeyFrame {
		glm::quat orientation;
	};

	struct AnimKeyScale : public AnimKeyFrame {
		glm::vec3 scale;
	};

	class AnimationChannel
	{
	public:
		AnimationChannel(const std::string& name, int ID, std::vector<AnimKeyPosition> positions, std::vector<AnimKeyRotation> rotations, std::vector<AnimKeyScale> scalings);
		~AnimationChannel();

		const glm::mat4& GetProcessedTransform() const { return processedTransform; }
		const std::string& GetChannelName() const { return name; }
		const int GetChannelID() const { return ID; }

		void Update(float animationTime);

		/*
		Gets the current index on 'positions' to interpolate to based on the current animation time

		Returns -1 upon error
		*/
		int GetPositionIndex(float animationTime);

		/*
		Gets the current index on 'rotations' to interpolate to based on the current animation time
		
		Returns -1 upon error
		*/
		int GetRotationIndex(float animationTime);

		/*
		Gets the current index on 'scalings' to interpolate to based on the current animation time
		
		Returns -1 upon error
		*/
		int GetScaleIndex(float animationTime);
	private:
		float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime);
		glm::mat4 InterpolatePosition(float animationTime);
		glm::mat4 InterpolateRotation(float animationTime);
		glm::mat4 InterpolateScaling(float animationTime);

		std::vector<AnimKeyPosition> positions;
		std::vector<AnimKeyRotation> rotations;
		std::vector<AnimKeyScale> scalings;

		glm::mat4 processedTransform;
		std::string name;
		int ID;
	};
}