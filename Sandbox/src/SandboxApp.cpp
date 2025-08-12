#include <PaulEngine.h>
#include <PaulEngine/Core/EntryPoint.h>

#include "Sandbox.h"

namespace Sandbox
{
	class SandboxApp : public PaulEngine::Application {
	public:
		SandboxApp(const PaulEngine::ApplicationSpecification& specification) : PaulEngine::Application(specification) {
			PE_PROFILE_FUNCTION();
			PushLayer(new Sandbox());
		}
		~SandboxApp() {}
	};
}
PaulEngine::Application* PaulEngine::CreateApplication(PaulEngine::ApplicationCommandLineArgs args) {
	ApplicationSpecification spec;
	spec.Name = "Paul Engine - Sandbox";
	spec.CommandLineArgs = args;

	return new Sandbox::SandboxApp(spec);
}