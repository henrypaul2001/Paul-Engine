#pragma once
#include <glad/glad.h>
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
namespace Engine {
	enum CameraMovement {
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT,
		UP,
		DOWN,
		UP_WORLD,
		DOWN_WORLD
	};
	
	class Camera
	{
	public:
		glm::vec3 Position;
		glm::vec3 Front;
		glm::vec3 Up;
		glm::vec3 Right;
		glm::vec3 WorldUp;

		float Yaw;
		float Pitch;

		float MovementSpeed;
		float MouseSensitivity;
		float Zoom;

		float NearClip;
		float FarClip;

		Camera(unsigned int scr_width, unsigned int scr_height, glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = -90.0f, float pitch = 0.0f, glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f), float moveSpeed = 6.5f, float mouseSens = 0.1f, float zoom = 45.0f);
		~Camera();

		glm::mat4 GetViewMatrix();
		glm::mat4 GetProjection();

		void ProcessKeyboard(CameraMovement direction, float deltaTime);
		void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);
		void ProcessMouseScroll(float yoffset);

	private:
		void UpdateCameraVectors();

		unsigned int SCR_WIDTH;
		unsigned int SCR_HEIGHT;
	};
}