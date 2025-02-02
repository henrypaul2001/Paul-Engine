#pragma once
#include "glm/vec3.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include <vector>
#include "Entity.h"
#include <glm/gtc/quaternion.hpp>
class EntityManagerNew;

namespace Engine
{
	class ComponentTransform
	{
	public:
		friend class EntityManager; // transform is a key component in the engine, therefore ECS and transform are tightly coupled and share a unique relationship

		//ComponentTransform(const ComponentTransform& old_component);
		ComponentTransform(EntityManager* owning_ecs, const glm::vec3& position, const glm::vec3& rotationAxis, const float rotationAngle, const glm::vec3& scale);
		ComponentTransform(EntityManager* owning_ecs, const glm::vec3& position);
		ComponentTransform(EntityManager* owning_ecs, const float posX, const float posY, const float posZ);
		~ComponentTransform();

		// Set

		void SetPosition(const glm::vec3& position) {
			this->position = position;
			UpdateModelMatrix();
		}
		void SetRotation(const glm::vec3& rotationAxis, const float rotationAngle) {
			this->rotationAxis = rotationAxis;
			this->rotationAngle = rotationAngle;
			SetOrientation(glm::angleAxis(glm::radians(rotationAngle), rotationAxis));
		}
		void SetScale(const glm::vec3& scale) {
			this->scale = scale;
			UpdateModelMatrix();
		}
		void SetScale(const float uniformScale) {
			this->scale = glm::vec3(uniformScale);
			UpdateModelMatrix();
		}
		void SetScale(const float xScale, const float yScale, const float zScale) {
			this->scale = glm::vec3(xScale, yScale, zScale);
			UpdateModelMatrix();
		}
		void SetOrientation(const glm::quat& orientation) {
			this->orientation = orientation;
			forwardVector = glm::normalize(orientation * glm::vec3(0.0f, 0.0f, 1.0f));
			UpdateModelMatrix();
		}

		// Get

		const glm::vec3& Position() const { return position; }
		const glm::vec3& RotationAxis() const { return rotationAxis; }
		const float RotationAngle() const { return rotationAngle; }
		const glm::vec3& Scale() const { return scale; }
		const glm::mat4& GetWorldModelMatrix() const { return worldModelMatrix; }
		const glm::vec3& GetForwardVector() const { return forwardVector; }
		const glm::vec3 GetWorldPosition() const;
		const float GetBiggestScaleFactor() const {
			float biggestScale = scale.x;
			if (scale.y > biggestScale) {
				biggestScale = scale.y;
			}
			else if (scale.z > biggestScale) {
				biggestScale = scale.z;
			}

			return biggestScale;
		}
		const glm::quat& GetOrientation() const { return orientation; }

		const std::vector<unsigned int>& GetChildren() const { return childrenIDs; }
		
		const Entity* FindChildWithName(const std::string& name) const;
		
		const unsigned int GetParent() const { return parentID; }

		void RemoveChild(const unsigned int entityID);
		void AddChild(const unsigned int entityID);

		void UpdateModelMatrix();
	private:
		ComponentTransform Clone() const;

		glm::vec3 position;
		glm::vec3 rotationAxis;
		float rotationAngle;
		glm::vec3 scale;
		glm::vec3 forwardVector;

		glm::quat orientation;

		glm::mat4 worldModelMatrix;

		unsigned int parentID;
		std::vector<unsigned int> childrenIDs;

		EntityManager* owning_ecs;
		unsigned int ownerID;
	};
}