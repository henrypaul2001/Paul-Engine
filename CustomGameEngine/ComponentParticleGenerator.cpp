#include "ComponentParticleGenerator.h"
namespace Engine {
	ComponentParticleGenerator::ComponentParticleGenerator(const ComponentParticleGenerator& old_component)
	{
		this->owner = nullptr;

		this->maxParticles = old_component.maxParticles;
		this->particlesPerSecond = old_component.particlesPerSecond;

		this->VAO = old_component.VAO;
		this->VBO = old_component.VBO;

		this->lastDeadParticle = old_component.lastDeadParticle;

		this->generatorVelocityScale = old_component.generatorVelocityScale;

		this->particleLifespan = old_component.particleLifespan;

		this->decayRate = old_component.decayRate;

		this->lessThanOneCount = old_component.lessThanOneCount;

		this->chanceToGenerateNewAcceleration = old_component.chanceToGenerateNewAcceleration;

		this->accelerationChangeScale = old_component.accelerationChangeScale;

		this->sphericalBillboarding = old_component.sphericalBillboarding;
		this->pointSprite = old_component.pointSprite;

		this->offset = old_component.offset;
		this->particleScale = old_component.particleScale;

		this->particles = old_component.particles;
		this->randomParams = old_component.randomParams;

		this->sprite = old_component.sprite;

		this->srcFactor = old_component.srcFactor;
		this->dstFactor = old_component.dstFactor;
	}

	ComponentParticleGenerator::ComponentParticleGenerator(Texture* sprite, unsigned int maxParticles, glm::vec3 offset, float particlesPerSecond, float particleLifespan, float decayRate, glm::vec3 particleScale, float velocityScale, float chanceToGenerateNewAccelerationPerParticle, float accelerationChangeScale, GLenum srcFactor, GLenum dstFactor)
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

		chanceToGenerateNewAcceleration = chanceToGenerateNewAccelerationPerParticle;
		this->accelerationChangeScale = accelerationChangeScale;

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