#pragma once
#include "System.h"
#include "ComponentTransform.h"
#include "ComponentParticleGenerator.h"
namespace Engine {
	class SystemParticleUpdater : public System
	{
    public:
        SystemParticleUpdater();
        ~SystemParticleUpdater();

        SystemTypes Name() override { return SYSTEM_PARTICLE_UPDATE; }
        void OnAction(Entity* entity) override;
        void AfterAction() override;

    private:
        const ComponentTypes MASK = (COMPONENT_TRANSFORM | COMPONENT_PARTICLE_GENERATOR);

        void UpdateParticles(ComponentTransform* transform, ComponentParticleGenerator* generator);

		float Random(float min, float max) const {
			return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
		}

		void SpawnParticle(Particle& particle, const ComponentParticleGenerator& generator, const glm::vec3& generatorPosition, const glm::vec3& generatorVelocity) {
			RandomParameters params = generator.GetRandomParameters();

			float randomXPosition = Random(params.randomPositionXRange.x, params.randomPositionXRange.y);
			float randomYPosition = Random(params.randomPositionYRange.x, params.randomPositionYRange.y);
			float randomZPosition = Random(params.randomPositionZRange.x, params.randomPositionZRange.y);

			float randomXVelocity = Random(params.randomVelocityXRange.x, params.randomVelocityXRange.y);
			float randomYVelocity = Random(params.randomVelocityYRange.x, params.randomVelocityYRange.y);
			float randomZVelocity = Random(params.randomVelocityZRange.x, params.randomVelocityZRange.y);

			float randomColour = Random(0.5f, 1.0f);

			glm::vec3 offset = generator.Offset();

			particle.Position = generatorPosition + offset + glm::vec3(randomXPosition, randomYPosition, randomZPosition);
			particle.Colour = glm::vec4(randomColour, randomColour, randomColour, 1.0f);
			particle.Life = generator.GetParticleLifespan();
			particle.Velocity = (generatorVelocity * generator.VelocityScale()) + glm::vec3(randomXVelocity, randomYVelocity, randomZVelocity);
			particle.Scale = generator.ParticleScale();
		}

        int FindFirstDeadParticle(const std::vector<Particle>& particles, const int lastDeadParticle) const {
			// Search from last dead particle, will usually return instantly
			for (unsigned int i = lastDeadParticle; i < particles.size(); i++) {
				if (particles[i].Life <= 0.0f) {
					return i;
				}
			}

			// Otherwise, search from beginning
			for (unsigned int i = 0; i < lastDeadParticle; i++) {
				if (particles[i].Life <= 0.0f) {
					return i;
				}
			}

			// All particles are alive
			return -1;
        }
	};
}