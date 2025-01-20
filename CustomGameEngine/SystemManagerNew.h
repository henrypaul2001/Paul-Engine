#pragma once
#include "EntityManagerNew.h"
#include <vector>
#include <functional>
#include <string>
namespace Engine {
	class SystemManagerNew
	{
	public:
		SystemManagerNew(EntityManagerNew* ecs) : ecs(ecs) {}
		~SystemManagerNew() {}

		template <typename... Components>
		bool RegisterSystem(const std::string& systemName, std::function<void(const unsigned int, Components&...)> onActionFunc, std::function<void()> afterActionFunc = []() {}) {
			if (systems.find(systemName) != systems.end()) {
				return false;
			}

			systems[systemName][0] = [this, onActionFunc]() {
				auto view = ecs->View<Components...>();
				view.ForEach(onActionFunc);
			};
			systems[systemName][1] = afterActionFunc;
			systemNames.push_back(systemName);
			return true;
		}

		void ActionSystems() const {
			for (const std::string& name : systemNames) {
				systems.at(name)[0]();
				systems.at(name)[1]();
			}
		}

	private:
		EntityManagerNew* ecs;

		std::vector<std::string> systemNames;
		std::unordered_map<std::string, std::function<void()>[2]> systems;
	};
}