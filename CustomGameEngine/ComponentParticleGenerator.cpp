#include "ComponentParticleGenerator.h"
namespace Engine {
	ComponentParticleGenerator::ComponentParticleGenerator(unsigned int maxParticles, glm::vec3 offset, unsigned int numberParticlesToRespawn, glm::vec3 particleScale, float velocityScale)
	{
		this->maxParticles = maxParticles;
		this->offset = offset;
		this->numberParticlesToRespawn = numberParticlesToRespawn;
		lastDeadParticle = 0;

		if (velocityScale > 1.0f) { velocityScale = 1.0f; }
		else if (velocityScale < -1.0f) { velocityScale = -1.0f; }

		this->generatorVelocityScale = velocityScale;
		this->particleScale = particleScale;

		particles.reserve(maxParticles);
		for (unsigned int i = 0; i < maxParticles; i++) {
			particles.push_back(Particle());
		}
	}

	ComponentParticleGenerator::~ComponentParticleGenerator()
	{

	}

	void ComponentParticleGenerator::Close()
	{
	}
}