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

		// should always initialise as paused as we dont currently know the position of the sound. Audio will be resumed in audio system where its correct position will also be set
		if (is3D) {
			sound = AudioManager::GetInstance()->GetSoundEngine()->play3D(activeAudio->GetSource(), irrklang::vec3df(0.0f, 0.0f, 0.0f), isLooped, true, true, enableSoundEffects);
		}
		else {
			sound = AudioManager::GetInstance()->GetSoundEngine()->play2D(activeAudio->GetSource(), isLooped, true, true, enableSoundEffects);
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