#pragma once
#include "PaulEngine/Renderer/Camera.h"

namespace PaulEngine
{
	enum SceneCameraType {
		SCENE_CAMERA_ORTHOGRAPHIC = 0,
		SCENE_CAMERA_PERSPECTIVE = 1
	};

	class SceneCamera : public Camera
	{
	public:
		SceneCamera(SceneCameraType type = SCENE_CAMERA_PERSPECTIVE);
		virtual ~SceneCamera();

		void SetPerspective(const float vfov = 90.0f, const float aspect = 1.777f, const float nearClip = 0.01f, const float farClip = 1000.0f);
		void SetOrthographic(float size = 1.0f, float nearClip = 0.01f, float farClip = 1000.0f);

		void SetViewportSize(uint32_t width, uint32_t height);

		inline bool IsPerspective() const { return m_Type; }

	private:
		float m_OrthographicSize;
		float m_NearClip, m_FarClip;

		float m_PerspectiveVFOV;

		float m_AspectRatio;

		SceneCameraType m_Type;
	};
}