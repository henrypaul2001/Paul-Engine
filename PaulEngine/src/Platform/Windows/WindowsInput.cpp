#include "pepch.h"
#include "PaulEngine/Core/Input.h"

#include "PaulEngine/Core/Application.h"
#include <GLFW/glfw3.h>

#ifdef PE_PLATFORM_WINDOWS
namespace PaulEngine {
	bool Input::IsKeyPressed(int keycode)
	{
		PE_PROFILE_FUNCTION();
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetKey(window, keycode);
		return (state == GLFW_PRESS || state == GLFW_REPEAT);
	}

	bool Input::IsMouseButtonPressed(int button)
	{
		PE_PROFILE_FUNCTION();
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetMouseButton(window, button);
		return (state == GLFW_PRESS);
	}

	std::pair<float, float> Input::GetMousePosition()
	{
		PE_PROFILE_FUNCTION();
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		return { (float)xpos, (float)ypos };
	}

	float Input::GetMouseX()
	{
		auto [x, y] = GetMousePosition();
		return x;
	}

	float Input::GetMouseY()
	{
		auto [x, y] = GetMousePosition();
		return y;
	}
}
#endif