#include "RuntimeApp.h"
#include "RuntimeMainLayer.h"

RuntimeApp::RuntimeApp(const PaulEngine::ApplicationSpecification& specification) : PaulEngine::Application(specification) {
	PE_PROFILE_FUNCTION();
	PushLayer(new RuntimeMainLayer());
}

RuntimeApp::~RuntimeApp() {}

PaulEngine::Application* PaulEngine::CreateApplication(PaulEngine::ApplicationCommandLineArgs args) {
	ApplicationSpecification spec;
	spec.Name = "Paul Engine - Runtime";
	spec.CommandLineArgs = args;

	return new RuntimeApp(spec);
}