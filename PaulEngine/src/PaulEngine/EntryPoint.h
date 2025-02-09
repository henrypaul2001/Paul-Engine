#pragma once
#include "Application.h"
#ifdef PE_PLATFORM_WINDOWS

extern PaulEngine::Application* PaulEngine::CreateApplication();

int main(int argc, char** argv) {
	auto app = PaulEngine::CreateApplication();
	app->Run();
	delete app;
}

#endif