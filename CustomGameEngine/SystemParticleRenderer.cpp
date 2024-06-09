#include "SystemParticleRenderer.h"
#include "ResourceManager.h"
namespace Engine {
	SystemParticleRenderer::SystemParticleRenderer()
	{

	}

	SystemParticleRenderer::~SystemParticleRenderer()
	{

	}

	void SystemParticleRenderer::OnAction(Entity* entity)
	{
		if ((entity->Mask() & MASK) == MASK) {
			std::vector<Component*> components = entity->Components();

			ComponentTransform* transform = nullptr;
			for (Component* c : components) {
				transform = dynamic_cast<ComponentTransform*>(c);
				if (transform != nullptr) {
					break;
				}
			}

			ComponentParticleGenerator* generator = nullptr;
			for (Component* c : components) {
				generator = dynamic_cast<ComponentParticleGenerator*>(c);
				if (generator != nullptr) {
					break;
				}
			}

			Draw(transform, generator);
		}
	}

	void SystemParticleRenderer::AfterAction()
	{

	}

	void SystemParticleRenderer::Draw(ComponentTransform* transform, ComponentParticleGenerator* generator)
	{
		Shader* particleShader;

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, generator->GetSprite()->id);

		glDisable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE); // additive blending

		std::vector<float> particleBuffer;
		particleBuffer.reserve(generator->MaxParticles() * 9);

		const std::vector<Particle> particles = generator->GetParticles();
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

		unsigned int VAO = generator->GetVAO();
		unsigned int VBO = generator->GetVBO();

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * particleBuffer.size(), particleBuffer.data(), GL_DYNAMIC_DRAW);

		// Draw particles
		particleShader = ResourceManager::GetInstance()->DefaultPointParticleShader();
		particleShader->Use();
		glDrawArraysInstanced(GL_POINTS, 0, 1, particles.size());

		glBindVertexArray(0);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_BLEND);
		glEnable(GL_CULL_FACE);
		glDepthMask(GL_TRUE);
	}
}