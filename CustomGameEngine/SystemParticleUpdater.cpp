#include "SystemParticleUpdater.h"
#include "Scene.h"
namespace Engine {
	SystemParticleUpdater::SystemParticleUpdater()
	{

	}

	SystemParticleUpdater::~SystemParticleUpdater()
	{

	}

	void SystemParticleUpdater::OnAction(Entity* entity)
	{
		if ((entity->Mask() & MASK) == MASK) {
			std::vector<Component*> components = entity->Components();

			ComponentTransform* transform = nullptr;
			for (Component* c : components) {
				transform = dynamic_cast<ComponentTransform*>(c);
				if (transform != nullptr) {
					break;
				}
			}

			ComponentParticleGenerator* generator = nullptr;
			for (Component* c : components) {
				generator = dynamic_cast<ComponentParticleGenerator*>(c);
				if (generator != nullptr) {
					break;
				}
			}

			UpdateParticles(transform, generator);
		}
	}

	void SystemParticleUpdater::AfterAction()
	{

	}

	void SystemParticleUpdater::UpdateParticles(ComponentTransform* transform, ComponentParticleGenerator* generator)
	{
		float deltaTime = Scene::dt;
		std::vector<Particle>& particles = generator->GetParticles();

		// Determine how many particles should be created during this frame based on the generators particles per second rate
		// If the result is less than 1 for this frame, accumulate this value into the next frame
		float particlesPerSecond = generator->ParticlesPerSecond();
		float previousFramesBuildup = generator->GetRunningLessThanOneCount();
		float fps = 1.0f / deltaTime;

		float particlesForThisFrame = particlesPerSecond / fps;
		unsigned int numberToRespawn = static_cast<unsigned int>(particlesForThisFrame);

		previousFramesBuildup += particlesForThisFrame - (float)numberToRespawn;

		if (previousFramesBuildup >= 1.0f) {
			numberToRespawn++;
			previousFramesBuildup -= 1.0f;
		}
		
		generator->SetRunningCount(previousFramesBuildup);

		// Get generator velocity
		glm::vec3 generatorVelocity = glm::vec3(0.0f);
		if ((generator->GetOwner()->Mask() & COMPONENT_PHYSICS) == COMPONENT_PHYSICS) {
			generatorVelocity = generator->GetOwner()->GetPhysicsComponent()->Velocity();
		}

		// Create new particles
		for (unsigned int i = 0; i < numberToRespawn; i++) {
			int firstDeadParticleIndex = FindFirstDeadParticle(particles, generator->LastDeadParticleIndex());
			if (firstDeadParticleIndex != -1) {
				generator->SetLastDeadParticleIndex(firstDeadParticleIndex);
				SpawnParticle(particles[firstDeadParticleIndex], *generator, transform->GetWorldPosition(), generatorVelocity);
			}
		}
		
		float decayRate = generator->GetDecayRate();
		float startingLifespan = generator->GetParticleLifespan();
		// Update particles
		for (Particle& p : particles) {
			p.Life -= decayRate * deltaTime;

			if (p.Life > 0.0f) {
				p.Velocity += p.Acceleration;
				p.Position += p.Velocity * deltaTime;
				p.Colour.a = 0.0f + (p.Life / startingLifespan);
			}
			else {
				p.Colour.a = 0.0f;
			}
		}
	}
}