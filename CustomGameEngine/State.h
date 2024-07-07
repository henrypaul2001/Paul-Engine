#pragma once
#include <string>
namespace Engine {
	class State
	{
	public:
		State(const std::string& name) { this->name = name; }
		~State() {}

		virtual void Update() = 0;

		const std::string& GetName() const { return name; }
	protected:
		std::string name;
	};
}