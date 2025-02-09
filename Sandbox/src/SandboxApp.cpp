#include <PaulEngine.h>

class Sandbox : public PaulEngine::Application {
public:
	Sandbox() {}
	~Sandbox() {}
};

PaulEngine::Application* PaulEngine::CreateApplication() {
	return new Sandbox();
}