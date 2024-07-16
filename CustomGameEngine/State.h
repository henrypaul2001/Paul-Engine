#pragma once
#include <string>
#include <iostream>
namespace Engine {
	class State
	{
	public:
		State(const std::string& name) { this->name = name; }
		~State() {}

		virtual void Update() = 0;
		virtual void Enter();
		virtual void Exit();

		const std::string& GetName() const { return name; }
	protected:
		std::string name;
	};
}