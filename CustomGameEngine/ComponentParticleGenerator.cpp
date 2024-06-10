#include "ComponentParticleGenerator.h"
namespace Engine {
	ComponentParticleGenerator::ComponentParticleGenerator(Texture* sprite, unsigned int maxParticles, glm::vec3 offset, float particlesPerSecond, float particleLifespan, float decayRate, glm::vec3 particleScale, float velocityScale, GLenum srcFactor, GLenum dstFactor)
	{
		this->sprite = sprite;
		this->maxParticles = maxParticles;
		this->offset = offset;

		if (particlesPerSecond < 0.0f) { particlesPerSecond = 0.0f; }
		this->particlesPerSecond = particlesPerSecond;

		this->srcFactor = srcFactor;
		this->dstFactor = dstFactor;
		this->particleLifespan = particleLifespan;
		this->decayRate = decayRate;
		lastDeadParticle = 0;
		lessThanOneCount = 0.0f;
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
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		// vertex attributes
		
		// v: pX, pY, pZ, sX, sY, cR, cG, cB, cA

		GLsizei stride = sizeof(float) * 9;

		// Position
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribDivisor(0, 1);

		// Scale
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 3));
		glEnableVertexAttribArray(1);
		glVertexAttribDivisor(1, 1);

		// Colour
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 5));
		glEnableVertexAttribArray(2);
		glVertexAttribDivisor(2, 1);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
}