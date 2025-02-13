#include <PaulEngine.h>
#include <imgui.h>

class TestLayer : public PaulEngine::Layer {
public:
	TestLayer() : Layer("Test Layer") {}

	void OnUpdate() override {
		//PE_INFO("TestLayer::Update");
		//auto [x, y] = PaulEngine::Input::GetMousePosition();
		//PE_TRACE("{0}, {1}", x, y);
	}

	void OnImGuiRender() override {
		ImGui::Begin("Test Window");
		ImGui::Text("Hello World!");
		ImGui::End();
	}

	void OnEvent(PaulEngine::Event& e) override {
		PE_TRACE(e);
		if (e.GetEventType() == PaulEngine::EventType::KeyPressed) {
			PaulEngine::KeyPressedEvent& event = (PaulEngine::KeyPressedEvent&)e;
			if (event.GetKeyCode() == PE_KEY_W) {
				PE_INFO("W key pressed");
			}
		}
	}
};

class Sandbox : public PaulEngine::Application {
public:
	Sandbox() {
		PushLayer(new TestLayer());
	}
	~Sandbox() {}
};

PaulEngine::Application* PaulEngine::CreateApplication() {
	return new Sandbox();
}