#pragma once
namespace Engine {
	class State
	{
	public:
		State() {}
		~State() {}

		virtual void Update() = 0;
	};
}