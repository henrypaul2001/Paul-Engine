#include "ComponentAudioSource.h"
#include "AudioManager.h"
namespace Engine {
	ComponentAudioSource::ComponentAudioSource(AudioFile* activeAudio, bool is3D, bool startPaused, bool isLooped, bool enableSoundEffects)
	{
		this->activeAudio = activeAudio;
		this->is3D = is3D;
		this->isLooped = isLooped;
		soundEffectsEnabled = enableSoundEffects;
		isPlaying = !startPaused;

		if (is3D) {
			sound = AudioManager::GetInstance()->GetSoundEngine()->play3D(activeAudio->GetSource(), irrklang::vec3df(0.0f, 0.0f, 0.0f), isLooped, startPaused, true, enableSoundEffects);
		}
		else {
			sound = AudioManager::GetInstance()->GetSoundEngine()->play2D(activeAudio->GetSource(), isLooped, startPaused, true, enableSoundEffects);
		}
	}

	ComponentAudioSource::~ComponentAudioSource()
	{
		sound->stop();
		sound->drop();
	}

	void ComponentAudioSource::Close()
	{

	}
}