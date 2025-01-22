#include "CollisionResolver.h"
namespace Engine {
	void CollisionResolver::Run(EntityManagerNew& ecs)
	{
		SCOPE_TIMER("CollisionResolver::Run");
		for (const CollisionData& collision : collisionManager->GetUnresolvedCollisions()) {
			const unsigned int entityIDA = collision.entityIDA;
			const unsigned int entityIDB = collision.entityIDB;

			ComponentTransform* transformA = ecs.GetComponent<ComponentTransform>(entityIDA);
			ComponentTransform* transformB = ecs.GetComponent<ComponentTransform>(entityIDB);

			ComponentPhysics* physicsA = ecs.GetComponent<ComponentPhysics>(entityIDA);
			ComponentPhysics* physicsB = ecs.GetComponent<ComponentPhysics>(entityIDB);

			ComponentCollision* colliderA = ecs.GetComponent<ComponentCollisionAABB>(entityIDA);
			if (!colliderA) { colliderA = ecs.GetComponent<ComponentCollisionSphere>(entityIDA); }
			if (!colliderA) { colliderA = ecs.GetComponent<ComponentCollisionBox>(entityIDA); }

			ComponentCollision* colliderB = ecs.GetComponent<ComponentCollisionAABB>(entityIDB);
			if (!colliderB) { colliderB = ecs.GetComponent<ComponentCollisionSphere>(entityIDB); }
			if (!colliderB) { colliderB = ecs.GetComponent<ComponentCollisionBox>(entityIDB); }

			float totalMass = 0.0f;
			if (physicsA) { totalMass += physicsA->InverseMass(); }
			if (physicsB) { totalMass += physicsB->InverseMass(); }

			// Seperate objects using projection
			Separate(transformA, physicsA, colliderA, transformB, physicsB, colliderB, totalMass, collision);

			// Update velocity of each physics component if they exist
			Impulse(transformA, physicsA, colliderA, transformB, physicsB, colliderB, totalMass, collision);

			//for (ContactPoint& contact : collision.contactPoints) {
				//PresolveContactPoint(contact, collision.objectA, collision.objectB, collision.contactPoints.size());
			//}

			//for (ContactPoint& contact : collision.contactPoints) {
				//SolveContactPoint(contact, collision.objectA, collision.objectB, collision.contactPoints.size());
			//}
		}

		collisionManager->ClearUnresolvedCollisions();
	}

	void CollisionResolver::Separate(ComponentTransform* transformA, ComponentPhysics* physicsA, ComponentCollision* colliderA, ComponentTransform* transformB, ComponentPhysics* physicsB, ComponentCollision* colliderB, float totalMass, const CollisionData& collision) const
	{
		for (const ContactPoint& contact : collision.contactPoints) {
			//ContactPoint& contact = collision.contactPoints[0];
			if (colliderA->IsMovedByCollisions()) {
				if (physicsA) {
					transformA->SetPosition(transformA->GetWorldPosition() + (contact.normal * contact.penetration) * (physicsA->InverseMass() / totalMass));
				}
				else {
					transformA->SetPosition(transformA->GetWorldPosition() + (contact.normal * contact.penetration));
				}
			}
			if (colliderB->IsMovedByCollisions()) {
				if (physicsB) {
					transformB->SetPosition(transformB->GetWorldPosition() - (contact.normal * contact.penetration) * (physicsB->InverseMass() / totalMass));
				}
				else {
					transformB->SetPosition(transformB->GetWorldPosition() - (contact.normal * contact.penetration));
				}
			}
		}
	}

	void CollisionResolver::Impulse(ComponentTransform* transformA, ComponentPhysics* physicsA, ComponentCollision* colliderA, ComponentTransform* transformB, ComponentPhysics* physicsB, ComponentCollision* colliderB, float totalMass, const CollisionData& collision) const
	{
		std::vector<glm::vec3> impulses;
		std::vector<glm::vec3> inertiaAs;
		std::vector<glm::vec3> inertiaBs;

		float elasticityA = 0.5f;
		float elasticityB = 0.5f;
		if (physicsA != nullptr) { elasticityA = physicsA->Elasticity(); }
		if (physicsB != nullptr) { elasticityB = physicsB->Elasticity(); }

		glm::vec3 startVelocityA = glm::vec3();
		glm::vec3 startVelocityB = glm::vec3();

		glm::vec3 startAngularA = glm::vec3();
		glm::vec3 startAngularB = glm::vec3();

		if (physicsA != nullptr) {
			startVelocityA = physicsA->Velocity();
			startAngularA = physicsA->AngularVelocity();
		}
		if (physicsB != nullptr) {
			startVelocityB = physicsB->Velocity();
			startAngularB = physicsB->AngularVelocity();
		}

		const glm::vec3 contactVelocity = startVelocityB - startVelocityA;

		for (const ContactPoint& contact : collision.contactPoints) {
			const glm::vec3 relativeA = contact.contactPointA;
			const glm::vec3 relativeB = contact.contactPointB;

			glm::vec3 relativeAngularA = glm::vec3();
			glm::vec3 relativeAngularB = glm::vec3();

			glm::vec3 relativeLinearA = startVelocityA;
			glm::vec3 relativeLinearB = startVelocityB;

			if (physicsA != nullptr) {
				relativeAngularA = glm::cross(startAngularA, relativeA);
				relativeLinearA += relativeAngularA;
			}
			if (physicsB != nullptr) {
				relativeAngularB = glm::cross(startAngularB, relativeB);
				relativeLinearB += relativeAngularB;
			}

			const float impulseForce = glm::dot(contactVelocity, contact.normal);

			glm::vec3 inertiaA = glm::vec3(0.0f);
			glm::vec3 inertiaB = glm::vec3(0.0f);
			if (physicsA != nullptr) { inertiaA = glm::cross(physicsA->InertiaTensor() * glm::cross(relativeA, contact.normal), relativeA); }
			if (physicsB != nullptr) { inertiaB = glm::cross(physicsB->InertiaTensor() * glm::cross(relativeB, contact.normal), relativeB); }

			const float angularEffect = glm::dot(inertiaA + inertiaB, contact.normal);

			const float coefficient = elasticityA * elasticityB;

			const float J = (-(1.0f + coefficient) * impulseForce) / (totalMass + angularEffect);
			const glm::vec3 fullImpulse = contact.normal * J;

			impulses.push_back(fullImpulse);

			inertiaAs.push_back(inertiaA);
			inertiaBs.push_back(inertiaB);
		}

		for (int i = 0; i < impulses.size(); i++) {
			//glm::vec3 impulse = impulses[i] / float(collision.contactPoints.size());
			const glm::vec3 impulse = impulses[i];
			const glm::vec3 relativeA = collision.contactPoints[i].contactPointA;
			const glm::vec3 relativeB = collision.contactPoints[i].contactPointB;

			if (physicsA && colliderA->IsMovedByCollisions()) {
				physicsA->ApplyLinearImpulse(-impulse / float(collision.contactPoints.size()));
				physicsA->ApplyAngularImpulse(glm::cross(relativeA, collision.contactPoints[i].normal * -impulse));
			}

			if (physicsB && colliderB->IsMovedByCollisions()) {
				physicsB->ApplyLinearImpulse(impulse / float(collision.contactPoints.size()));
				physicsB->ApplyAngularImpulse(glm::cross(relativeB, collision.contactPoints[i].normal * impulse));
			}
		}
	}

	/*
	void CollisionResolver::PresolveContactPoint(ContactPoint& contact, Entity* objectA, Entity* objectB, int numContacts)
	{
		// Update contact constraint
		contact.sumImpulseContact = 0.0f;
		contact.sumImpulseFriction = glm::vec3(0.0f);
		contact.b_term = 0.0f;

		// Baumgarte offset
		const float baumgarte_scalar = 0.1f;
		const float baumgart_slop = 0.001f;
		const float penetration_slop = std::min(contact.penetration + baumgart_slop, 0.0f);

		contact.b_term += -(baumgarte_scalar / Scene::dt) * penetration_slop;

		// Elasticity
		ComponentPhysics* physicsA = objectA->GetPhysicsComponent();
		ComponentPhysics* physicsB = objectB->GetPhysicsComponent();

		float elasticityA = 0.0f;
		float elasticityB = 0.0f;

		glm::vec3 linearA = glm::vec3(0.0f);
		glm::vec3 angularA = glm::vec3(0.0f);

		glm::vec3 linearB = glm::vec3(0.0f);
		glm::vec3 angularB = glm::vec3(0.0f);

		if (physicsA != nullptr) {
			elasticityA = physicsA->Elasticity();
			linearA = physicsA->Velocity();
			angularA = physicsA->AngularVelocity();
		}
		if (physicsB != nullptr) { 
			elasticityB = physicsB->Elasticity();
			linearB = physicsB->Velocity();
			angularB = physicsB->AngularVelocity();
		}
		const float elasticity = elasticityA * elasticityB;

		const float elasticity_term = glm::dot(contact.normal,
			linearA + glm::cross(contact.contactPointA, angularA) - linearB - glm::cross(contact.contactPointB, angularB));

		contact.b_term += (elasticity * elasticity_term) / numContacts;
	}

	void CollisionResolver::SolveContactPoint(ContactPoint& contact, Entity* objectA, Entity* objectB, int numContacts)
	{
		ComponentPhysics* physicsA = objectA->GetPhysicsComponent();
		ComponentPhysics* physicsB = objectB->GetPhysicsComponent();

		glm::vec3 linearA = glm::vec3(0.0f);
		glm::vec3 angularA = glm::vec3(0.0f);

		glm::vec3 linearB = glm::vec3(0.0f);
		glm::vec3 angularB = glm::vec3(0.0f);

		float inverseMassA = 0.0f;
		float inverseMassB = 0.0f;

		float frictionA = 1.0f;
		float frictionB = 1.0f;

		glm::vec3 inertiaA = glm::vec3(0.0f);
		glm::vec3 inertiaB = glm::vec3(0.0f);

		if (physicsA != nullptr) {
			linearA = physicsA->Velocity();
			angularA = physicsA->AngularVelocity();
			inverseMassA = physicsA->InverseMass();
			//frictionA = physicsA->Friction();
		}
		if (physicsB != nullptr) {
			linearB = physicsB->Velocity();
			angularB = physicsB->AngularVelocity();
			inverseMassB = physicsB->InverseMass();
			//frictionB = physicsB->Friction();
		}

		float totalInverseMass = inverseMassA + inverseMassB;

		glm::vec3 r1 = contact.contactPointA;
		glm::vec3 r2 = contact.contactPointB;

		glm::vec3 v0 = linearA + glm::cross(angularA, r1);
		glm::vec3 v1 = linearB + glm::cross(angularB, r2);

		glm::vec3 dv = v1 - v0;

		if (physicsA != nullptr) { inertiaA = glm::cross(physicsA->InertiaTensor() * glm::cross(r1, contact.normal), r1); }
		if (physicsB != nullptr) { inertiaB = glm::cross(physicsB->InertiaTensor() * glm::cross(r2, contact.normal), r2); }
		
		// Resolve
		float constraintMass = totalInverseMass + glm::dot(inertiaA + inertiaB, contact.normal);

		if (constraintMass > 0.0f) {
			float jn = std::max(-glm::dot(dv, contact.normal) + contact.b_term, 0.0f);
			jn = jn / constraintMass;

			if (physicsA != nullptr) {
				physicsA->SetVelocity(linearA - ((contact.normal * (jn * inverseMassA)) / float(numContacts)));
				physicsA->SetAngularVelocity(physicsA->AngularVelocity() - physicsA->InertiaTensor() * glm::cross(r1, contact.normal * jn));
			}

			if (physicsB != nullptr) {
				physicsB->SetVelocity(linearB + ((contact.normal * (jn * inverseMassB)) / float(numContacts)));
				physicsB->SetAngularVelocity(physicsB->AngularVelocity() + physicsB->InertiaTensor() * glm::cross(r2, contact.normal * jn));
			}
		}

		// Friction
		/*
		glm::vec3 tangent = dv - contact.normal * glm::dot(dv, contact.normal);
		float tangentLength = glm::length(tangent);

		if (tangentLength > 1e-6f) {
			tangent = tangent / tangentLength;
			float frictionalMass = totalInverseMass + glm::dot(tangent, inertiaA + inertiaB);

			if (frictionalMass > 0.0f) {
				float frictionCoefficient = frictionA * frictionB;
				float jt = -glm::dot(dv, tangent) * frictionCoefficient;

				jt = jt / frictionalMass;

				if (physicsA != nullptr) {
					physicsA->SetVelocity(linearA - tangent * (jt * inverseMassA));
					physicsA->SetAngularVelocity(physicsA->AngularVelocity() - physicsA->InverseInertiaTensor() * glm::cross(r1, tangent * jt));
				}

				if (physicsB != nullptr) {
					physicsB->SetVelocity(linearB + tangent * (jt * inverseMassB));
					physicsB->SetAngularVelocity(physicsB->AngularVelocity() + physicsB->InverseInertiaTensor() * glm::cross(r2, tangent * jt));
				}
			}
		}
		
	}
	*/
}