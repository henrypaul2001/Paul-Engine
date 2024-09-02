#include "ConstraintSolver.h"
#include "Scene.h"
namespace Engine {
	void ConstraintSolver::OnAction()
	{
		SCOPE_TIMER("ConstraintSolver::OnAction");
		std::vector<Constraint*> constraints = constraintManager->GetConstraints();
		float dividedDeltaTime = Scene::dt / float(numIterations);

		for (int i = 0; i < numIterations; i++) {
			for (Constraint* c : constraints) {
				if (c->IsActive()) {
					c->UpdateConstraint(dividedDeltaTime);
				}
			}
		}
	}

	void ConstraintSolver::AfterAction()
	{

	}
}