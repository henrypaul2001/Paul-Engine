#pragma once
#include "ConstraintManager.h"
#include "EntityManagerNew.h"
namespace Engine {
	class ConstraintSolver
	{
	public:
		ConstraintSolver(EntityManagerNew* ecs, ConstraintManager* constraintManager, const int numIterations = 4) : ecs(ecs), constraintManager(constraintManager), numIterations(numIterations) {}
		~ConstraintSolver() {}

		void Run();
		void AfterAction();

		void SetNumberOfIterations(const int newIterations) { numIterations = newIterations; }
		int NumberOfIterations() const { return numIterations; }
	private:
		EntityManagerNew* ecs;
		ConstraintManager* constraintManager;
		int numIterations;
	};
}