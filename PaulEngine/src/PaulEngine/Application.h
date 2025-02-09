#pragma once
#include "Core.h"

namespace PaulEngine {
	class PAUL_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
	};
}