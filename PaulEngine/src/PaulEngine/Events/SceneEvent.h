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
}