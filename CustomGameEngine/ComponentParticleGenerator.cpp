#include "ComponentParticleGenerator.h"
namespace Engine {
	ComponentParticleGenerator::ComponentParticleGenerator(Texture* sprite, unsigned int maxParticles, glm::vec3 offset, unsigned int numberParticlesToRespawn, glm::vec3 particleScale, float velocityScale)
	{
		this->sprite = sprite;
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

		SetupBuffers();
	}

	ComponentParticleGenerator::~ComponentParticleGenerator()
	{

	}

	void ComponentParticleGenerator::Close()
	{
	}

	void ComponentParticleGenerator::SetupBuffers()
	{
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		// Position
		glGenBuffers(1, &positionVBO);
		glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribDivisor(0, 1);

		// Scale
		glGenBuffers(1, &scaleVBO);
		glBindBuffer(GL_ARRAY_BUFFER, scaleVBO);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribDivisor(1, 1);

		// Colour
		glGenBuffers(1, &colourVBO);
		glBindBuffer(GL_ARRAY_BUFFER, colourVBO);
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
		glEnableVertexAttribArray(2);
		glVertexAttribDivisor(2, 1);
	}
}