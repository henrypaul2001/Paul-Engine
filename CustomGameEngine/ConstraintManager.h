#pragma once
#include <vector>
#include "Constraint.h"
namespace Engine {
	class ConstraintManager
	{
	public:
		ConstraintManager() {}
		~ConstraintManager() { ClearConstraints(); }

		const Constraint* GetConstraint(const unsigned int index) const { return constraints[index]; }
		void AddNewConstraint(Constraint* constraint) { constraints.push_back(constraint); }

		void RemoveConstraint(Constraint* constraintToRemove) {
			constraints.erase(std::find(constraints.begin(), constraints.end(), constraintToRemove));
			delete constraintToRemove;
		}
		void RemoveConstraint(int indexToRemove) { 
			Constraint* c = constraints[indexToRemove];
			constraints.erase(constraints.begin() + indexToRemove);
			delete c;
		}

		void ClearConstraints() {
			for (Constraint* c : constraints) { delete c; }
			constraints.clear();
		}

		std::vector<Constraint*>& GetConstraints() { return constraints; }
	protected:
		std::vector<Constraint*> constraints;
	};
}