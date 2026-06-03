#pragma once
#ifdef PE_PLATFORM_WINDOWS

extern PaulEngine::Application* PaulEngine::CreateApplication(ApplicationCommandLineArgs args);

int main(int argc, char** argv) {

	PE_PROFILE_BEGIN_SESSION("Startup", "PaulEngineProfile-Startup.json");
	PaulEngine::Log::Init();
	PE_CORE_TRACE("Welcome to Paul Engine.");
	auto app = PaulEngine::CreateApplication({argc, argv});
	PE_PROFILE_END_SESSION();

	PE_PROFILE_BEGIN_SESSION("Runtime", "PaulEngineProfile-Runtime.json");
	app->Run();
	PE_PROFILE_END_SESSION();

	PE_PROFILE_BEGIN_SESSION("Shutdown", "PaulEngineProfile-Shutdown.json");
	delete app;
	PE_PROFILE_END_SESSION();
}

#endif