#include <PaulEngine.h>

class TestLayer : public PaulEngine::Layer {
public:
	TestLayer() : Layer("Test Layer") {}

	void OnUpdate() override {
		PE_INFO("TestLayer::Update");
	}

	void OnEvent(PaulEngine::Event& e) override {
		PE_TRACE(e);
	}
};

class Sandbox : public PaulEngine::Application {
public:
	Sandbox() {
		PushLayer(new TestLayer());
		PushOverlay(new PaulEngine::ImGuiLayer());
	}
	~Sandbox() {}
};

PaulEngine::Application* PaulEngine::CreateApplication() {
	return new Sandbox();
}