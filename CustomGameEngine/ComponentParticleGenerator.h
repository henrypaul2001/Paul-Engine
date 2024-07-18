#pragma once
#include "Component.h"
#include "Mesh.h"
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
namespace Engine {

	struct Particle {
		glm::vec3 Position, Velocity, Scale, Acceleration;
		glm::vec4 Colour;
		float Life;

		Particle() : Position(0.0f), Velocity(0.0f), Scale(0.0f), Colour(0.0f), Life(0.0f), Acceleration(0.0f) {}
	};

	struct RandomParameters {
		glm::vec2 randomPositionXRange;
		glm::vec2 randomPositionYRange;
		glm::vec2 randomPositionZRange;

		glm::vec2 randomVelocityXRange;
		glm::vec2 randomVelocityYRange;
		glm::vec2 randomVelocityZRange;

		glm::vec2 randomAccelerationXRange;
		glm::vec2 randomAccelerationYRange;
		glm::vec2 randomAccelerationZRange;

		RandomParameters() : randomPositionXRange(-0.5f, 0.5f), randomPositionYRange(-0.5f, 0.5f), randomPositionZRange(-0.5f, 0.5f), randomVelocityXRange(-0.5f, 0.5f), randomVelocityYRange(-0.5f, 0.5f), randomVelocityZRange(-0.5f, 0.5f), randomAccelerationXRange(0.0f), randomAccelerationYRange(0.0f), randomAccelerationZRange(0.0f) {}
	};

	class ComponentParticleGenerator : public Component
	{
	public:
		ComponentParticleGenerator(const ComponentParticleGenerator& old_component);
		ComponentParticleGenerator(Texture* sprite, unsigned int maxParticles = 500, glm::vec3 offset = glm::vec3(0.0f), float particlesPerSecond = 2.0f, float particleLifespan = 1.0f, float decayRate = 0.5f, glm::vec3 particleScale = glm::vec3(1.0f), float velocityScale = 0.5f, float chanceToGenerateNewAccelerationPerParticle = 0.0f, float accelerationChangeScale = 1.0f, GLenum srcFactor = GL_SRC_ALPHA, GLenum dstFactor = GL_ONE);
		~ComponentParticleGenerator();

		ComponentTypes ComponentType() override { return COMPONENT_PARTICLE_GENERATOR; }
		void Close() override;

		Component* Copy() override { return new ComponentParticleGenerator(*this); }

		unsigned int MaxParticles() const { return maxParticles; }
		const float ParticlesPerSecond() const { return particlesPerSecond; }
		int LastDeadParticleIndex() const { return lastDeadParticle; }
		const glm::vec3& Offset() const { return offset; }
		float VelocityScale() const { return generatorVelocityScale; }
		const glm::vec3& ParticleScale() const { return particleScale; }

		const unsigned int GetVAO() const { return VAO; }
		const unsigned int GetVBO() const { return VBO; }
		const GLenum GetSrcBlendFactor() const { return srcFactor; }
		const GLenum GetDstBlendFactor() const { return dstFactor; }

		const float GetParticleLifespan() const { return particleLifespan; }
		const float GetDecayRate() const { return decayRate; }

		const float GetRunningLessThanOneCount() const { return lessThanOneCount; }

		const float GetChanceToGenerateNewAcceleration() const { return chanceToGenerateNewAcceleration; }
		const float GetAccelerationChangeScale() const { return accelerationChangeScale; }

		void SetLastDeadParticleIndex(int newValue) { lastDeadParticle = newValue; }
		void SetRandomParameters(RandomParameters params) { randomParams = params; }

		void SetSrcBlendFactor(const GLenum srcBlend) { srcFactor = srcBlend; }
		void SetDstBlendFactor(const GLenum dstBlend) { dstFactor = dstBlend; }

		void SetParticleLifespan(const float lifespan) { particleLifespan = lifespan; }
		void SetDecayRate(const float decayRate) { this->decayRate = decayRate; }
		
		void SetRunningCount(const float count) { this->lessThanOneCount = count; }

		void SetAccelerationChangeChance(const float chance) { chanceToGenerateNewAcceleration = chance; }
		void SetAccelerationChangeScale(const float scale) { accelerationChangeScale = scale; }

		std::vector<Particle>& GetParticles() { return particles; }
		const RandomParameters& GetRandomParameters() const { return randomParams; }

		const Texture* GetSprite() const { return sprite; }
		void SetSprite(Texture* newSprite) { sprite = newSprite; }
	private:
		void SetupBuffers();

		unsigned int maxParticles;
		float particlesPerSecond;

		// For instanced rendering
		unsigned int VAO, VBO;

		int lastDeadParticle;

		// Scale -1.0, 1.0. Applies to particle velocity when taking into account velocity of generator
		float generatorVelocityScale;

		float particleLifespan;

		float decayRate;
		
		float lessThanOneCount;

		float chanceToGenerateNewAcceleration;

		// If the acceleration of a particle gets regenerated during its lifespan, the resulting acceleration will be multiplied by this number
		float accelerationChangeScale;

		// If false, cylindrical billboarding will be used instead
		bool sphericalBillboarding;
		bool pointSprite;

		glm::vec3 offset;
		glm::vec3 particleScale;

		std::vector<Particle> particles;
		RandomParameters randomParams;

		Texture* sprite;

		GLenum srcFactor;
		GLenum dstFactor;
	};
}