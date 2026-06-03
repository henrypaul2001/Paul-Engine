#include <PaulEngine.h>
#include <PaulEngine/Core/EntryPoint.h>

#include "EditorLayer.h"

namespace PaulEngine {
	class EditorApp : public Application {
	public:
		EditorApp(const ApplicationSpecification& specification) : Application(specification) {
			PE_PROFILE_FUNCTION();
			PushLayer(new EditorLayer());
		}
		~EditorApp() {}
	};

	Application* CreateApplication(ApplicationCommandLineArgs args) {
		ApplicationSpecification spec;
		spec.Name = "Paul Engine - Editor";
		spec.CommandLineArgs = args;

		return new EditorApp(spec);
	}
}