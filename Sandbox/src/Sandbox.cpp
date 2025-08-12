#include "Sandbox.h"
#include <imgui.h>

namespace Sandbox
{
	Sandbox::Sandbox() : Layer("Sandbox_Layer") {}

	Sandbox::~Sandbox() {}

	void Sandbox::OnAttach()
	{
		PE_PROFILE_FUNCTION();
		Math::Vector3 up = Math::Vec3(0.0f, 1.0f, 0.0f);
		Math::Vector3 right = Math::Vec3(1.0f, 0.0f, 0.0f);
		Math::Vector3 front = Math::Cross(up, right);
		PE_CORE_TRACE("Front = {0}",front);

		Math::Vec3 normal = Math::Vec3(0.0f, 0.0f, -1.0f);
		Math::Vec3 fragPos = Math::Vec3(0.0f, 0.0f, 0.0f);
		Math::Vec3 lightPos = Math::Vec3(1.0f, 0.0f, -5.0f);
		Math::Vec3 fragToLight = Math::Normalize(lightPos - fragPos);

		float contribution = Math::Dot(fragToLight, normal);
		contribution = std::max(0.0f, contribution);
		contribution = std::min(1.0f, contribution);
		PE_CORE_TRACE("Light pos = {0}", lightPos);
		PE_CORE_TRACE("Frag pos = {0}", fragPos);
		PE_CORE_TRACE("Surface normal = {0}", normal);
		PE_CORE_TRACE("Direction to light = {0}", fragToLight);
		PE_CORE_TRACE("Light contribution = {0}", contribution);
	}

	void Sandbox::OnDetach()
	{
		PE_PROFILE_FUNCTION();
	}


	static PaulEngine::Timestep deltaTime = 0.0f;
	void Sandbox::OnUpdate(const PaulEngine::Timestep timestep)
	{
		PE_PROFILE_FUNCTION();
		deltaTime = timestep;
		PaulEngine::RenderCommand::SetClearColour(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
		PaulEngine::RenderCommand::Clear();
	}

	void Sandbox::OnImGuiRender()
	{
		PE_PROFILE_FUNCTION();

		const PaulEngine::Renderer2D::Statistics& stats = PaulEngine::Renderer2D::GetStats();
		ImGui::Begin("Renderer2D");
		ImGui::SeparatorText("Renderer2D Stats:");
		ImGui::Text("Draw Calls: %d", stats.DrawCalls);
		ImGui::Text("Quad Count: %d", stats.QuadCount);
		ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
		ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
		ImGui::Text("Timestep (ms): %f", deltaTime.GetMilliseconds());
		ImGui::Text("FPS: %d", (int)(1.0f / deltaTime.GetSeconds()));
		ImGui::End();
	}

	void Sandbox::OnEvent(PaulEngine::Event& e)
	{
		PE_PROFILE_FUNCTION();

		PaulEngine::EventDispatcher dispatcher = PaulEngine::EventDispatcher(e);
		dispatcher.DispatchEvent<PaulEngine::KeyReleasedEvent>(PE_BIND_EVENT_FN(Sandbox::OnKeyUp));
	}

	bool Sandbox::OnKeyUp(PaulEngine::KeyReleasedEvent& e)
	{
		if (e.GetKeyCode() == PE_KEY_ESCAPE) { PaulEngine::Application::Get().Close(); return true; }
	}
}