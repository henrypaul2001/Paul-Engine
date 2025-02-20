#pragma once
#include "pepch.h"
#include "PaulEngine/Events/Event.h"

namespace PaulEngine {
	struct WindowProperties {
		std::string Title;
		unsigned int Width, Height;

		WindowProperties(const std::string& title = "Paul Engine", unsigned int width = 1280, unsigned int height = 720) : Title(title), Width(width), Height(height) {}
	};

	// Interface for platform specific windows
	class Window {
	public:
		using EventCallbackFunction = std::function<void(Event&)>;
		virtual ~Window() = default;

		virtual void OnUpdate() = 0;

		virtual unsigned int GetWidth() const = 0;
		virtual unsigned int GetHeight() const = 0;

		virtual void SetEventCallback(const EventCallbackFunction& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		virtual void* GetNativeWindow() const = 0;

		static Window* Create(const WindowProperties& properties = WindowProperties());
	};
}