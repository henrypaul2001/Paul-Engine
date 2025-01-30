#include "SystemParticleUpdater.h"
#include "ComponentPhysics.h"
#include "Scene.h"
namespace Engine {
	void SystemParticleUpdater::OnAction(const unsigned int entityID, ComponentTransform& transform, ComponentParticleGenerator& generator)
	{
		SCOPE_TIMER("SystemParticleUpdater::OnAction()");
		const float deltaTime = Scene::dt;
		std::vector<Particle>& particles = generator.GetParticles();

		// Determine how many particles should be created during this frame based on the generators particles per second rate
		// If the result is less than 1 for this frame, accumulate this value into the next frame
		const float particlesPerSecond = generator.ParticlesPerSecond();
		float previousFramesBuildup = generator.GetRunningLessThanOneCount();
		const float fps = 1.0f / deltaTime;

		const float particlesForThisFrame = particlesPerSecond / fps;
		unsigned int numberToRespawn = static_cast<unsigned int>(particlesForThisFrame);

		previousFramesBuildup += particlesForThisFrame - (float)numberToRespawn;

		if (previousFramesBuildup >= 1.0f) {
			numberToRespawn++;
			previousFramesBuildup -= 1.0f;
		}

		generator.SetRunningCount(previousFramesBuildup);

		// Get generator velocity
		glm::vec3 generatorVelocity = glm::vec3(0.0f);
		const ComponentPhysics* physics = active_ecs->GetComponent<ComponentPhysics>(entityID);
		if (physics) { generatorVelocity = physics->Velocity(); }

		// Create new particles
		for (unsigned int i = 0; i < numberToRespawn; i++) {
			const int firstDeadParticleIndex = FindFirstDeadParticle(particles, generator.LastDeadParticleIndex());
			if (firstDeadParticleIndex != -1) {
				generator.SetLastDeadParticleIndex(firstDeadParticleIndex);
				SpawnParticle(particles[firstDeadParticleIndex], generator, transform.GetWorldPosition(), generatorVelocity);
			}
		}

		const float decayRate = generator.GetDecayRate();
		const float startingLifespan = generator.GetParticleLifespan();

		const float accelerationChangeProbability = generator.GetChanceToGenerateNewAcceleration();
		const float acclerationChangeScale = generator.GetAccelerationChangeScale();
		const RandomParameters& params = generator.GetRandomParameters();

		// Update particles
		for (Particle& p : particles) {
			p.Life -= decayRate * deltaTime;

			if (p.Life > 0.0f) {
				p.Velocity += p.Acceleration;
				p.Position += p.Velocity * deltaTime;
				p.Colour.a = 0.0f + (p.Life / startingLifespan);

				if (Random(0.0f, 1.0f) < accelerationChangeProbability) {
					// Generate new acceleration based on random paramaters of generator
					const float randomXAcceleration = Random(params.randomAccelerationXRange.x, params.randomAccelerationXRange.y);
					const float randomYAcceleration = Random(params.randomAccelerationYRange.x, params.randomAccelerationYRange.y);
					const float randomZAcceleration = Random(params.randomAccelerationZRange.x, params.randomAccelerationZRange.y);
					p.Acceleration = glm::vec3(randomXAcceleration, randomYAcceleration, randomZAcceleration) * acclerationChangeScale;
				}
			}
			else {
				p.Colour.a = 0.0f;
			}
		}
	}

	void SystemParticleUpdater::AfterAction() {}
}