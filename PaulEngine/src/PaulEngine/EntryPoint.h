#pragma once
#ifdef PE_PLATFORM_WINDOWS

extern PaulEngine::Application* PaulEngine::CreateApplication();

int main(int argc, char** argv) {
	PaulEngine::Log::Init();
	PE_CORE_WARN("Warning log");
	int a = 5;
	PE_INFO("Hello! Var={0}", a);

	auto app = PaulEngine::CreateApplication();
	app->Run();
	delete app;
}

#endif