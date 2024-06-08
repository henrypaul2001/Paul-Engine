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
		Shader* particleShader = ResourceManager::GetInstance()->DefaultParticleShader();

		particleShader->Use();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, generator->GetSprite()->id);

		glDisable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE); // additive blending

		particleShader->setBool("sphericalBillboarding", generator->SphericalBillboarding());

		glm::mat4 model = glm::mat4(1.0f);
		for (const Particle& particle : generator->GetParticles()) {
			model = glm::mat4(1.0f);
			model = glm::translate(model, particle.Position);
			model = glm::scale(model, particle.Scale);
			particleShader->setMat4("model", model);
			particleShader->setVec4("colour", particle.Colour);
			glBindVertexArray(ResourceManager::GetInstance()->DefaultPlane().VAO);
			glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(ResourceManager::GetInstance()->DefaultPlane().indices.size()), GL_UNSIGNED_INT, 0);
		}

		glBindVertexArray(0);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_BLEND);
		glEnable(GL_CULL_FACE);
		glDepthMask(GL_TRUE);
	}
}