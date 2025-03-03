#include "pepch.h"
#include "PaulEngine/Scene/SceneCamera.h"

namespace PaulEngine
{
	SceneCamera::SceneCamera(SceneCameraType type) : m_OrthographicSize(10.0f), m_NearClip(0.01f), m_FarClip(1000.0f), m_PerspectiveVFOV(90.0f), m_AspectRatio(1.777f), m_Type(type)
	{
		SetViewportSize(16, 9);
	}

	SceneCamera::~SceneCamera()
	{

	}

	void SceneCamera::SetPerspective(const float vfov, const float aspect, const float nearClip, const float farClip)
	{
		PE_PROFILE_FUNCTION();
		m_Type = SCENE_CAMERA_PERSPECTIVE;
		m_NearClip = nearClip;
		m_FarClip = farClip;
		m_AspectRatio = aspect;
		m_PerspectiveVFOV = vfov;

		m_Projection = glm::perspective(m_PerspectiveVFOV, m_AspectRatio, m_NearClip, m_FarClip);
	}

	void SceneCamera::SetOrthographic(float size, float nearClip, float farClip)
	{
		PE_PROFILE_FUNCTION();
		m_Type = SCENE_CAMERA_ORTHOGRAPHIC;
		m_NearClip = nearClip;
		m_FarClip = farClip;
		m_OrthographicSize = size;

		float orthoLeft = -m_OrthographicSize * m_AspectRatio * 0.5f;
		float orthoRight = m_OrthographicSize * m_AspectRatio * 0.5f;
		float orthoBottom = -m_OrthographicSize * 0.5f;
		float orthoTop = m_OrthographicSize * 0.5f;

		m_Projection = glm::ortho(orthoLeft, orthoRight, orthoBottom, orthoTop);
	}

	void SceneCamera::SetViewportSize(uint32_t width, uint32_t height)
	{
		m_AspectRatio = (float)width / (float)height;

		if (IsPerspective()) { SetPerspective(m_PerspectiveVFOV, m_AspectRatio, m_NearClip, m_FarClip); }
		else { SetOrthographic(m_OrthographicSize, m_NearClip, m_FarClip); }
	}
}