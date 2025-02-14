#pragma once
#include "PaulEngine/Events/Event.h"

namespace PaulEngine {
	class KeyEvent : public Event {
	public:
		inline int GetKeyCode() const { return m_Keycode; }
		inline int GetScancode() const { return m_Scancode; }

		EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)

	protected:
		KeyEvent(int keycode, int scancode) : m_Keycode(keycode), m_Scancode(scancode) {}

		int m_Keycode;
		int m_Scancode;
	};

	class KeyPressedEvent : public KeyEvent {
	public:
		KeyPressedEvent(int keycode, int scancode, int repeatCount) : KeyEvent(keycode, scancode), m_RepeatCount(repeatCount) {}

		inline int GetRepeatCount() const { return m_RepeatCount; }

		std::string ToString() const override {
			std::stringstream ss;
			ss << "KeyPressedEvent: " << m_Keycode << " (" << m_RepeatCount << " repeats)";
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyPressed)
	private:
		int m_RepeatCount;
	};

	class KeyReleasedEvent : public KeyEvent {
	public:
		KeyReleasedEvent(int keycode, int scancode) : KeyEvent(keycode, scancode) {}

		std::string ToString() const override {
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << m_Keycode;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyReleased)
	};

	class KeyTypedEvent : public KeyEvent {
	public:
		KeyTypedEvent(int keycode, int scancode) : KeyEvent(keycode, scancode) {}

		std::string ToString() const override {
			std::stringstream ss;
			ss << "KeyTypeEvent: " << m_Keycode;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyTyped)

	};
}