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

		std::vector<glm::vec3> positions;
		std::vector<glm::vec2> scales;
		std::vector<glm::vec4> colours;

		for (const Particle& particle : generator->GetParticles()) {
			positions.push_back(particle.Position);
			scales.push_back(glm::vec2(particle.Scale.x, particle.Scale.y));
			colours.push_back(particle.Colour);
		}

		unsigned int VAO = generator->GetVAO();
		unsigned int positionVBO = generator->GetPositionVBO();
		unsigned int scaleVBO = generator->GetScaleVBO();
		unsigned int colourVBO = generator->GetColourVBO();

		glBindVertexArray(VAO);

		// Position buffer
		glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
		glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_DYNAMIC_DRAW);

		// Scale buffer
		glBindBuffer(GL_ARRAY_BUFFER, scaleVBO);
		glBufferData(GL_ARRAY_BUFFER, scales.size() * sizeof(glm::vec2), scales.data(), GL_DYNAMIC_DRAW);

		// Colour buffer
		glBindBuffer(GL_ARRAY_BUFFER, colourVBO);
		glBufferData(GL_ARRAY_BUFFER, colours.size() * sizeof(glm::vec4), colours.data(), GL_DYNAMIC_DRAW);

		// Draw particles
		particleShader = ResourceManager::GetInstance()->DefaultPointParticleShader();
		particleShader->Use();
		glDrawArraysInstanced(GL_POINTS, 0, 1, positions.size());

		glBindVertexArray(0);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_BLEND);
		glEnable(GL_CULL_FACE);
		glDepthMask(GL_TRUE);
	}
}