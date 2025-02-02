#pragma once
#include "EntityManager.h"
#include <vector>
#include <functional>
#include "ScopeTimer.h"
#include <string>
namespace Engine {
	class SystemManager
	{
	public:
		SystemManager(EntityManager* ecs) : ecs(ecs) {}
		~SystemManager() {}

		template <typename... Components>
		bool RegisterSystem(const std::string& systemName, std::function<void(const unsigned int, Components&...)> onActionFunc, std::function<void()> preActionFunc = []() {}, std::function<void()> afterActionFunc = []() {}) {
			if (systems.find(systemName) != systems.end()) {
				return false;
			}

			systems[systemName][0] = preActionFunc;
			systems[systemName][1] = [this, onActionFunc]() {
				auto view = ecs->View<Components...>();
				view.ForEach(onActionFunc);
			};
			systems[systemName][2] = afterActionFunc;
			systemNames.push_back(systemName);
			return true;
		}

		void ActionSystems() const {
			SCOPE_TIMER("SystemManager::ActionSystems()");
			for (const std::string& name : systemNames) {
				const std::string scopeName = "SystemManager::ActionSystems::" + name + "::Run()";
				SCOPE_TIMER(scopeName.c_str());
				systems.at(name)[0]();
				systems.at(name)[1]();
				systems.at(name)[2]();
			}
		}

		template <typename... Components>
		bool RegisterPreUpdateSystem(const std::string& systemName, std::function<void(const unsigned int, Components&...)> onActionFunc, std::function<void()> preActionFunc = []() {}, std::function<void()> afterActionFunc = []() {}) {
			if (preUpdateSystems.find(systemName) != preUpdateSystems.end()) {
				return false;
			}

			preUpdateSystems[systemName][0] = preActionFunc;
			preUpdateSystems[systemName][1] = [this, onActionFunc]() {
				auto view = ecs->View<Components...>();
				view.ForEach(onActionFunc);
				};
			preUpdateSystems[systemName][2] = afterActionFunc;
			preUpdateSystemNames.push_back(systemName);
			return true;
		}

		void ActionPreUpdateSystems() const {
			SCOPE_TIMER("SystemManager::ActionPreUpdateSystems()");
			for (const std::string& name : preUpdateSystemNames) {
				const std::string scopeName = "SystemManager::ActionPreUpdateSystems::" + name + "::Run()";
				SCOPE_TIMER(scopeName.c_str());
				preUpdateSystems.at(name)[0]();
				preUpdateSystems.at(name)[1]();
				preUpdateSystems.at(name)[2]();
			}
		}

	private:
		EntityManager* ecs;

		std::vector<std::string> preUpdateSystemNames;
		std::unordered_map<std::string, std::function<void()>[3]> preUpdateSystems;

		std::vector<std::string> systemNames;
		std::unordered_map<std::string, std::function<void()>[3]> systems;
	};
}