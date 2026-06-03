#pragma once
#include "pepch.h"
#include "PaulEngine/Events/Event.h"

namespace PaulEngine {
	struct WindowProperties {
		std::string Title;
		uint32_t Width, Height;

		WindowProperties(const std::string& title = "Paul Engine", uint32_t width = 1280, uint32_t height = 720) : Title(title), Width(width), Height(height) {}
	};

	// Interface for platform specific windows
	class Window {
	public:
		using EventCallbackFunction = std::function<void(Event&)>;
		virtual ~Window() = default;

		virtual void OnUpdate() = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual int GetPosX() const = 0;
		virtual int GetPosY() const = 0;

		virtual void SetEventCallback(const EventCallbackFunction& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		virtual void* GetNativeWindow() const = 0;

		static Window* Create(const WindowProperties& properties = WindowProperties());
	};
}