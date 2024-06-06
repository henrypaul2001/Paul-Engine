#include "ComponentParticleGenerator.h"
namespace Engine {
	ComponentParticleGenerator::ComponentParticleGenerator(unsigned int maxParticles, glm::vec3 offset)
	{
		this->maxParticles = maxParticles;
		this->offset = offset;

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