#include <PaulEngine.h>
#include <PaulEngine/Core/EntryPoint.h>

#include "EditorLayer.h"

namespace PaulEngine {
	class EditorApp : public Application {
	public:
		EditorApp() : Application("Paul Engine - Editor") {
			PE_PROFILE_FUNCTION();
			PushLayer(new EditorLayer());
		}
		~EditorApp() {}
	};

	Application* CreateApplication() {
		return new EditorApp();
	}
}