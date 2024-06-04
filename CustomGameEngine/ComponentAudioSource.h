#pragma once
#include "Component.h"
#include "AudioFile.h"
#include "irrklang/irrklang.h"
namespace Engine {
	class ComponentAudioSource : public Component
	{
	public:
		ComponentAudioSource(AudioFile* activeAudio, bool is3D = true, bool startPaused = false, bool isLooped = true, bool enableSoundEffects = true);
		~ComponentAudioSource();

		ComponentTypes ComponentType() override { return COMPONENT_AUDIO_SOURCE; }
		void Close() override;

		bool Is3D() const { return is3D; }
		void Set3D(bool newValue) { is3D = newValue; }

		bool IsLooped() const { return isLooped; }
		void SetIsLooped(bool newValue) { isLooped = newValue; }

		bool IsPlaying() const { return isPlaying; }
		void Pause() { isPlaying = false; }
		void Play() { isPlaying = true; }

		AudioFile* GetAudioFile() const { return activeAudio; }
		void SetActiveAudio(AudioFile* newAudio, bool startPaused) {
			irrklang::vec3df previousPosition = sound->getPosition();
			activeAudio = newAudio;

			if (is3D) {
				sound = AudioManager::GetInstance()->GetSoundEngine()->play3D(activeAudio->GetSource(), previousPosition, isLooped, startPaused, true, soundEffectsEnabled);
			}
			else {
				sound = AudioManager::GetInstance()->GetSoundEngine()->play2D(activeAudio->GetSource(), isLooped, startPaused, true, soundEffectsEnabled);
			}
		}
	private:
		bool is3D;
		bool isPlaying;
		bool isLooped;
		bool soundEffectsEnabled;

		irrklang::ISound* sound;
		AudioFile* activeAudio;
	};
}