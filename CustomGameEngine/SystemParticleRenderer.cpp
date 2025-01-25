#include "SystemParticleRenderer.h"
#include "ResourceManager.h"
namespace Engine {
	void SystemParticleRenderer::OnAction(const unsigned int entityID, ComponentTransform& transform, ComponentParticleGenerator& generator)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, generator.GetSprite()->id);

		glDisable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glBlendFunc(generator.GetSrcBlendFactor(), generator.GetDstBlendFactor()); // additive blending

		std::vector<float> particleBuffer;
		particleBuffer.reserve(generator.MaxParticles() * 9);

		const std::vector<Particle>& particles = generator.GetParticles();
		for (const Particle& particle : particles) {
			particleBuffer.push_back(particle.Position.x);
			particleBuffer.push_back(particle.Position.y);
			particleBuffer.push_back(particle.Position.z);

			particleBuffer.push_back(particle.Scale.x);
			particleBuffer.push_back(particle.Scale.y);

			particleBuffer.push_back(particle.Colour.r);
			particleBuffer.push_back(particle.Colour.g);
			particleBuffer.push_back(particle.Colour.b);
			particleBuffer.push_back(particle.Colour.a);
		}

		const unsigned int VAO = generator.GetVAO();
		const unsigned int VBO = generator.GetVBO();

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * particleBuffer.size(), particleBuffer.data(), GL_DYNAMIC_DRAW);

		// Draw particles
		Shader* particleShader = ResourceManager::GetInstance()->DefaultPointParticleShader();
		particleShader->Use();
		glDrawArraysInstanced(GL_POINTS, 0, 1, particles.size());

		glBindVertexArray(0);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_BLEND);
		glEnable(GL_CULL_FACE);
		glDepthMask(GL_TRUE);
	}
}