#include "ConstraintSolver.h"
#include "Scene.h"
namespace Engine {
	void ConstraintSolver::Run(EntityManagerNew& ecs)
	{
		SCOPE_TIMER("ConstraintSolver::Run");
		const std::vector<Constraint*>& constraints = constraintManager->GetConstraints();
		float dividedDeltaTime = Scene::dt / float(numIterations);

		for (int i = 0; i < numIterations; i++) {
			for (const Constraint* c : constraints) {
				if (c->IsActive()) {
					c->UpdateConstraint(ecs, dividedDeltaTime);
				}
			}
		}
	}

	void ConstraintSolver::AfterAction()
	{

	}
}