#include "SystemAudio.h"
namespace Engine {
	SystemAudio::SystemAudio()
	{

	}

	SystemAudio::~SystemAudio()
	{

	}

	void SystemAudio::Run(const std::vector<Entity*>& entityList)
	{
		SCOPE_TIMER("SystemAudio::Run");
		System::Run(entityList);
	}

	void SystemAudio::OnAction(Entity* entity)
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

			ComponentAudioSource* audio = nullptr;
			for (Component* c : components) {
				audio = dynamic_cast<ComponentAudioSource*>(c);
				if (audio != nullptr) {
					break;
				}
			}

			UpdateAudio(transform, audio);
		}
	}

	void SystemAudio::AfterAction()
	{

	}

	void SystemAudio::UpdateAudio(ComponentTransform* transform, ComponentAudioSource* audio)
	{
		glm::vec3 position = transform->GetWorldPosition();
		irrklang::ISound* sound = audio->GetSound();

		if (audio->Is3D()) {
			sound->setPosition(irrklang::vec3df(position.x, position.y, position.z));
		}

		sound->setIsPaused(!audio->IsPlaying());
		sound->setIsLooped(audio->IsLooped());
	}
}