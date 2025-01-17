#include "ComponentAudioSource.h"
namespace Engine {
	ComponentAudioSource::ComponentAudioSource(const ComponentAudioSource& old_component)
	{
		this->is3D = old_component.is3D;
		this->isPlaying = old_component.isPlaying;
		this->isLooped = old_component.isLooped;
		this->soundEffectsEnabled = old_component.soundEffectsEnabled;
		this->activeAudio = old_component.activeAudio;

		// should always initialise as paused as we dont currently know the position of the sound. Audio will be resumed in audio system where its correct position will also be set
		if (is3D) {
			sound = AudioManager::GetInstance()->GetSoundEngine()->play3D(activeAudio->GetSource(), irrklang::vec3df(0.0f, 0.0f, 0.0f), isLooped, true, true, soundEffectsEnabled);
		}
		else {
			sound = AudioManager::GetInstance()->GetSoundEngine()->play2D(activeAudio->GetSource(), isLooped, true, true, soundEffectsEnabled);
		}

		sfxController = nullptr;
		if (sound) {
			sfxController = sound->getSoundEffectControl();
		}
	}

	ComponentAudioSource::ComponentAudioSource(AudioFile* activeAudio, const bool is3D, const bool startPaused, const bool isLooped, const bool enableSoundEffects)
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

		sfxController = nullptr;
		if (sound) {
			sfxController = sound->getSoundEffectControl();
		}
	}

	ComponentAudioSource::~ComponentAudioSource()
	{
		sound->stop();
		sound->drop();
	}
}