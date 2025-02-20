#pragma once
#ifdef PE_PLATFORM_WINDOWS

extern PaulEngine::Application* PaulEngine::CreateApplication();

int main(int argc, char** argv) {
	PaulEngine::Log::Init();
	PE_CORE_TRACE("Welcome to Paul Engine.");
	auto app = PaulEngine::CreateApplication();
	app->Run();
	delete app;
}

#endif