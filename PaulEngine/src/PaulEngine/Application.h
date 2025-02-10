#pragma once
#include "Core.h"
#include "Events/Event.h"

namespace PaulEngine {
	class PAUL_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
	};

	// To be defined by client
	Application* CreateApplication();
}