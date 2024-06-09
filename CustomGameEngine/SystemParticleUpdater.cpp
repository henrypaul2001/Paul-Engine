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

		unsigned int framesSinceLastRespawn = generator->FramesSinceLastRespawn();
		unsigned int delay = generator->RespawnDelay();
		unsigned int numberToRespawn = generator->NumberParticlesToRespawn();

		// Spawn new particles
		if (framesSinceLastRespawn == delay) {
			generator->SetFramesSinceLastRespawn(0);

			// Create new particles
			for (unsigned int i = 0; i < numberToRespawn; i++) {
				int firstDeadParticleIndex = FindFirstDeadParticle(particles, generator->LastDeadParticleIndex());
				if (firstDeadParticleIndex != -1) {
					generator->SetLastDeadParticleIndex(firstDeadParticleIndex);
					SpawnParticle(particles[firstDeadParticleIndex], *generator, transform->GetWorldPosition(), glm::vec3(0.0f));
				}
			}
		}
		else {
			generator->SetFramesSinceLastRespawn(++framesSinceLastRespawn);
		}

		float startingLifespan = generator->GetParticleLifespan();
		// Update particles
		for (Particle& p : particles) {
			p.Life -= deltaTime;

			if (p.Life > 0.0f) {
				p.Position += p.Velocity * deltaTime;
				p.Colour.a = 0.0f + (p.Life / startingLifespan);
			}
		}
	}
}