#pragma once
#include "Component.h"
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
namespace Engine {

	struct Particle {
		glm::vec3 Position, Velocity, Scale;
		glm::vec4 Colour;
		float Life;

		Particle() : Position(0.0f), Velocity(0.0f), Scale(1.0f), Colour(1.0f), Life(0.0f) {}
	};

	class ComponentParticleGenerator : public Component
	{
	public:
		ComponentParticleGenerator(unsigned int maxParticles = 500, glm::vec3 offset = glm::vec3(0.0f));
		~ComponentParticleGenerator();

		ComponentTypes ComponentType() override { return COMPONENT_PARTICLE_GENERATOR; }
		void Close() override;

		unsigned int MaxParticles() const { return maxParticles; }
		unsigned int RespawnDelay() const { return respawnDelay; }
		unsigned int FramesSinceLastRespawn() const { return framesSinceLastRespawn; }

		void SetFramesSinceLastRespawn(unsigned int newValue) { framesSinceLastRespawn = newValue; }

		std::vector<Particle>& GetParticles() { return particles; }
	private:
		unsigned int maxParticles;
		unsigned int respawnDelay;
		unsigned int framesSinceLastRespawn;

		glm::vec3 offset;

		std::vector<Particle> particles;
	};
}