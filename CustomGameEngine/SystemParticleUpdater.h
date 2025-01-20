#pragma once
#include "SystemNew.h"
#include "ComponentTransform.h"
#include "ComponentParticleGenerator.h"
namespace Engine {
	class SystemParticleUpdater : public SystemNew
	{
    public:
        SystemParticleUpdater(EntityManagerNew* ecs) : SystemNew(ecs) {}
        ~SystemParticleUpdater() {}

		constexpr const char* SystemName() override { return "SYSTEM_PARTICLE_UPDATER"; }

        void OnAction(const unsigned int entityID, ComponentTransform& transform, ComponentParticleGenerator& generator);
        void AfterAction();

    private:
		float Random(float min, float max) const { return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min))); }

		void SpawnParticle(Particle& particle, const ComponentParticleGenerator& generator, const glm::vec3& generatorPosition, const glm::vec3& generatorVelocity) const {
			const RandomParameters& params = generator.GetRandomParameters();

			const float randomXPosition = Random(params.randomPositionXRange.x, params.randomPositionXRange.y);
			const float randomYPosition = Random(params.randomPositionYRange.x, params.randomPositionYRange.y);
			const float randomZPosition = Random(params.randomPositionZRange.x, params.randomPositionZRange.y);

			const float randomXVelocity = Random(params.randomVelocityXRange.x, params.randomVelocityXRange.y);
			const float randomYVelocity = Random(params.randomVelocityYRange.x, params.randomVelocityYRange.y);
			const float randomZVelocity = Random(params.randomVelocityZRange.x, params.randomVelocityZRange.y);

			const float randomXAcceleration = Random(params.randomAccelerationXRange.x, params.randomAccelerationXRange.y);
			const float randomYAcceleration = Random(params.randomAccelerationYRange.x, params.randomAccelerationYRange.y);
			const float randomZAcceleration = Random(params.randomAccelerationZRange.x, params.randomAccelerationZRange.y);

			const float randomColour = Random(0.5f, 1.0f);

			const glm::vec3& offset = generator.Offset();

			particle.Position = generatorPosition + offset + glm::vec3(randomXPosition, randomYPosition, randomZPosition);
			particle.Colour = glm::vec4(randomColour, randomColour, randomColour, 1.0f);
			particle.Life = generator.GetParticleLifespan();
			particle.Velocity = (generatorVelocity * generator.VelocityScale()) + glm::vec3(randomXVelocity, randomYVelocity, randomZVelocity);
			particle.Scale = generator.ParticleScale();
			particle.Acceleration = glm::vec3(randomXAcceleration, randomYAcceleration, randomZAcceleration);
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