#pragma once
#include "PaulEngine/Layer.h"
#include <PaulEngine/Events/MouseEvent.h>
#include <PaulEngine/Events/KeyEvent.h>

struct GLFWwindow;

namespace PaulEngine {
	class PAUL_API ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate() override;
		void OnEvent(Event& e) override;

		bool OnMousePressed(MouseButtonPressedEvent& e);
		bool OnMouseReleased(MouseButtonReleasedEvent& e);
		bool OnMouseScroll(MouseScrolledEvent& e);
		bool OnKeyDown(KeyPressedEvent& e);
		bool OnKeyUp(KeyReleasedEvent& e);
		bool OnMouseMoved(MouseMovedEvent& e);

	private:
		float m_Time = 0.0f;

		static void ImGui_ImplGlfw_UpdateKeyModifiers(GLFWwindow* window);
	};
}