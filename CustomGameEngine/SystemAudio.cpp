#include "SystemAudio.h"
namespace Engine {
	void SystemAudio::OnAction(const unsigned int entityID, ComponentTransform& transform, ComponentAudioSource& audio)
	{
		glm::vec3 position = transform.GetWorldPosition();
		irrklang::ISound* sound = audio.GetSound();

		if (audio.Is3D()) {
			sound->setPosition(irrklang::vec3df(position.x, position.y, position.z));
		}

		sound->setIsPaused(!audio.IsPlaying());
		sound->setIsLooped(audio.IsLooped());
	}

	void SystemAudio::AfterAction() {}
}