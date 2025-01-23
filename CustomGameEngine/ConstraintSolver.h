#pragma once
#include "ConstraintManager.h"
#include "EntityManagerNew.h"
namespace Engine {
	class ConstraintSolver
	{
	public:
		ConstraintSolver(ConstraintManager* constraintManager, const int numIterations = 4) : constraintManager(constraintManager), numIterations(numIterations) {}
		~ConstraintSolver() {}

		void Run(EntityManagerNew& ecs);
		void AfterAction();

		void SetNumberOfIterations(const int newIterations) { numIterations = newIterations; }
		int NumberOfIterations() const { return numIterations; }
	private:
		ConstraintManager* constraintManager;
		int numIterations;
	};
}