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
			ComponentTransform* transform = entity->GetTransformComponent();
			ComponentAudioSource* audio = entity->GetAudioComponent();

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