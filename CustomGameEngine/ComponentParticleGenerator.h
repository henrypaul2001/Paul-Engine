#pragma once
#include "Component.h"
#include "Mesh.h"
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
namespace Engine {

	struct Particle {
		glm::vec3 Position, Velocity, Scale;
		glm::vec4 Colour;
		float Life;

		Particle() : Position(0.0f), Velocity(0.0f), Scale(0.0f), Colour(0.0f), Life(0.0f) {}
	};

	struct RandomParameters {
		glm::vec2 randomPositionXRange;
		glm::vec2 randomPositionYRange;
		glm::vec2 randomPositionZRange;

		glm::vec2 randomVelocityXRange;
		glm::vec2 randomVelocityYRange;
		glm::vec2 randomVelocityZRange;

		RandomParameters() : randomPositionXRange(-0.5f, 0.5f), randomPositionYRange(-0.5f, 0.5f), randomPositionZRange(-0.5f, 0.5f), randomVelocityXRange(-0.5f, 0.5f), randomVelocityYRange(-0.5f, 0.5f), randomVelocityZRange(-0.5f, 0.5f) {}
	};

	class ComponentParticleGenerator : public Component
	{
	public:
		ComponentParticleGenerator(Texture* sprite, unsigned int maxParticles = 500, glm::vec3 offset = glm::vec3(0.0f), unsigned int numberParticlesToRespawn = 2, float particleLifespan = 1.0f, glm::vec3 particleScale = glm::vec3(1.0f), float velocityScale = 0.5f, GLenum srcFactor = GL_SRC_ALPHA, GLenum dstFactor = GL_ONE);
		~ComponentParticleGenerator();

		ComponentTypes ComponentType() override { return COMPONENT_PARTICLE_GENERATOR; }
		void Close() override;

		unsigned int MaxParticles() const { return maxParticles; }
		unsigned int RespawnDelay() const { return respawnDelay; }
		unsigned int FramesSinceLastRespawn() const { return framesSinceLastRespawn; }
		unsigned int NumberParticlesToRespawn() const { return numberParticlesToRespawn; }
		int LastDeadParticleIndex() const { return lastDeadParticle; }
		const glm::vec3& Offset() const { return offset; }
		float VelocityScale() const { return generatorVelocityScale; }
		const glm::vec3& ParticleScale() const { return particleScale; }

		const unsigned int GetVAO() const { return VAO; }
		const unsigned int GetVBO() const { return VBO; }
		const GLenum GetSrcBlendFactor() const { return srcFactor; }
		const GLenum GetDstBlendFactor() const { return dstFactor; }

		const float GetParticleLifespan() const { return particleLifespan; }

		void SetFramesSinceLastRespawn(unsigned int newValue) { framesSinceLastRespawn = newValue; }
		void SetLastDeadParticleIndex(int newValue) { lastDeadParticle = newValue; }
		void SetRandomParameters(RandomParameters params) { randomParams = params; }

		void SetSrcBlendFactor(const GLenum srcBlend) { srcFactor = srcBlend; }
		void SetDstBlendFactor(const GLenum dstBlend) { dstFactor = dstBlend; }

		void SetParticleLifespan(const float lifespan) { particleLifespan = lifespan; }
		void SetRespawnDelay(const unsigned int frameDelay) { respawnDelay = frameDelay; }

		std::vector<Particle>& GetParticles() { return particles; }
		const RandomParameters& GetRandomParameters() const { return randomParams; }

		const Texture* GetSprite() const { return sprite; }
		void SetSprite(Texture* newSprite) { sprite = newSprite; }
	private:
		void SetupBuffers();

		unsigned int maxParticles;
		unsigned int respawnDelay;
		unsigned int framesSinceLastRespawn;
		unsigned int numberParticlesToRespawn;

		// For instanced rendering
		unsigned int VAO, VBO;

		int lastDeadParticle;

		// Scale -1.0, 1.0. Applies to particle velocity when taking into account velocity of generator
		float generatorVelocityScale;

		float particleLifespan;

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