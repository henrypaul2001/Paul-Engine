#pragma once
namespace PaulEngine {
	class GraphicsContext {
	public:
		virtual ~GraphicsContext() {}
		virtual void Init() = 0;
		virtual void SwapBuffers() = 0;
	};
}