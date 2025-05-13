#pragma once
#include "PaulEngine/Events/Event.h"

namespace PaulEngine
{
	class SceneShouldChangeEvent : public Event
	{
	public:
		SceneShouldChangeEvent(AssetHandle sceneHandle) : m_SceneHandle(sceneHandle) {}

		inline AssetHandle GetSceneHandle() const { return m_SceneHandle; }

		std::string ToString() const override {
			std::stringstream ss;
			ss << "SceneShouldChangeEvent: " << m_SceneHandle;
			return ss.str();
		}

		EVENT_CLASS_TYPE(SceneShouldChange)
		EVENT_CLASS_CATEGORY(EventCategoryScene)

	private:
		AssetHandle m_SceneHandle;
	};

	class SceneChangedEvent : public Event
	{
	public:
		SceneChangedEvent(AssetHandle sceneHandle) : m_SceneHandle(sceneHandle) {}

		inline AssetHandle GetSceneHandle() const { return m_SceneHandle; }

		std::string ToString() const override {
			std::stringstream ss;
			ss << "SceneChangedEvent: " << m_SceneHandle;
			return ss.str();
		}

		EVENT_CLASS_TYPE(SceneChanged)
		EVENT_CLASS_CATEGORY(EventCategoryScene)

	private:
		AssetHandle m_SceneHandle;
	};

	class MainViewportResizeEvent : public Event {
	public:
		MainViewportResizeEvent(uint32_t width, uint32_t height) : m_Width(width), m_Height(height) {}

		uint32_t GetWidth() const { return m_Width; }
		uint32_t GetHeight() const { return m_Height; }

		std::string ToString() const override {
			std::stringstream ss;
			ss << "MainViewportResizeEvent: " << m_Width << " x " << m_Height;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MainViewportResize)
		EVENT_CLASS_CATEGORY(EventCategoryScene)
	private:
		uint32_t m_Width, m_Height;
	};
}