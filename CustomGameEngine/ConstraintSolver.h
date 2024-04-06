#pragma once
#include "ConstraintManager.h"
namespace Engine {
	class ConstraintSolver
	{
	public:
		ConstraintSolver(ConstraintManager* constraintManager, int numIterations = 4) : constraintManager(constraintManager), numIterations(numIterations) {}
		~ConstraintSolver() {}

		void OnAction();
		void AfterAction();

		void SetNumberOfIterations(int newIterations) { numIterations = newIterations; }
	private:
		ConstraintManager* constraintManager;
		int numIterations;
	};
}