#pragma once
#include <vector>
#include "Constraint.h"
namespace Engine {
	class ConstraintManager
	{
	public:
		ConstraintManager() {}
		~ConstraintManager() {}

		void AddNewConstraint(Constraint* constraint) { constraints.push_back(constraint); }

		void RemoveConstraint(Constraint* constraintToRemove) {
			constraints.erase(std::find(constraints.begin(), constraints.end(), constraintToRemove));
			delete constraintToRemove;
		}
		void RemoveConstraint(int indexToRemove) {
			Constraint* constraintToRemove = constraints[indexToRemove];
			constraints.erase(constraints.begin() + indexToRemove);
			delete constraintToRemove;
		}

		void ClearConstraints() {
			for (int i = 0; i < constraints.size(); i++) {
				delete constraints[i];
			}
			constraints.clear();
		}

		std::vector<Constraint*>& GetConstraints() { return constraints; }
	protected:
		std::vector<Constraint*> constraints;
	};
}