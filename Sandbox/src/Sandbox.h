#pragma once
#include <PaulEngine.h>
#include <PaulEngine/OrthographicCameraController.h>

#include "Maths/Vector.h"

namespace Sandbox
{
	class Sandbox : public PaulEngine::Layer
	{
	public:
		Sandbox();
		~Sandbox();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(const PaulEngine::Timestep timestep) override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(PaulEngine::Event& e) override;

	private:
		bool OnKeyUp(PaulEngine::KeyReleasedEvent& e);
	};
}