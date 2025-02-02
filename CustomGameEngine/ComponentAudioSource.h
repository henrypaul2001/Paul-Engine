#pragma once
#include "AudioFile.h"
#include "AudioManager.h"
namespace Engine {
	class ComponentAudioSource
	{
	public:
		ComponentAudioSource(const ComponentAudioSource& old_component);
		ComponentAudioSource(AudioFile* activeAudio, const bool is3D = true, const bool startPaused = false, const bool isLooped = true, const bool enableSoundEffects = true);
		~ComponentAudioSource();

		bool Is3D() const { return is3D; }
		void Set3D(bool newValue) { 
			is3D = newValue;

			irrklang::vec3df previousPosition = sound->getPosition();

			sound->stop();
			sound->drop();

			if (is3D) {
				sound = AudioManager::GetInstance()->GetSoundEngine()->play3D(activeAudio->GetSource(), previousPosition, isLooped, !isPlaying, true, soundEffectsEnabled);
			}
			else {
				sound = AudioManager::GetInstance()->GetSoundEngine()->play2D(activeAudio->GetSource(), isLooped, !isPlaying, true, soundEffectsEnabled);
			}
		}

		bool IsLooped() const { return isLooped; }
		void SetIsLooped(bool newValue) { isLooped = newValue; }

		bool IsPlaying() const { return isPlaying; }
		void Pause() { isPlaying = false; }
		void Play() { isPlaying = true; }

		AudioFile* GetAudioFile() const { return activeAudio; }
		void SetActiveAudio(AudioFile* newAudio, bool startPaused) {
			irrklang::vec3df previousPosition = sound->getPosition();
			activeAudio = newAudio;

			sound->stop();
			sound->drop();

			if (is3D) {
				sound = AudioManager::GetInstance()->GetSoundEngine()->play3D(activeAudio->GetSource(), previousPosition, isLooped, startPaused, true, soundEffectsEnabled);
			}
			else {
				sound = AudioManager::GetInstance()->GetSoundEngine()->play2D(activeAudio->GetSource(), isLooped, startPaused, true, soundEffectsEnabled);
			}
		}

		irrklang::ISound* GetSound() { return sound; }
		irrklang::ISoundEffectControl* GetSFXController() { return sfxController; }
	private:
		bool is3D;
		bool isPlaying;
		bool isLooped;
		bool soundEffectsEnabled;

		irrklang::ISoundEffectControl* sfxController;
		irrklang::ISound* sound;
		AudioFile* activeAudio;
	};
}