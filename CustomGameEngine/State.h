#pragma once
#include <string>
#include <iostream>
#include "EntityManager.h"
namespace Engine {
	class StateMachine;
	class ComponentStateController;

	class State
	{
	public:
		State(const std::string& name) { this->name = name; }
		~State() {}

		virtual void Update(EntityManager* ecs, const unsigned int entityID) = 0;
		virtual void Enter(EntityManager* ecs, const unsigned int entityID);
		virtual void Exit(EntityManager* ecs, const unsigned int entityID);

		virtual State* Copy() = 0;

		const std::string& GetName() const { return name; }

		const StateMachine* GetParentStateMachine() const { return parentStateMachine; }
		void SetParentStateMachine(StateMachine* newParent) { parentStateMachine = newParent; }
	protected:
		std::string name;

		StateMachine* parentStateMachine;
	};
}