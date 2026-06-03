#pragma once
#include <PaulEngine.h>

class RuntimeApp : public PaulEngine::Application
{
public:
	RuntimeApp(const PaulEngine::ApplicationSpecification& specification);
	~RuntimeApp();
};

