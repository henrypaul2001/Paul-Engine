#pragma once
#include "irrKlang/irrklang.h"
#include <string>
namespace Engine {

	// default audio settings used for every sound source using this audio file
	struct AudioFileDefaultSettings {
		float defaultMaxAttenuationDistance; // after this distance, audio no longer attenuates and stays at the same volume
		float defaultMinAttenuationDistance; // distance in which audio should begin attenuating, distances lower than this will be full volume
		float defaultVolume; // base volume of audio file
		float defaultPan; // -1 = left || 0 = center || 1 = right
	};

	class AudioFile
	{
	public:
		AudioFile(irrklang::ISoundSource* source, float defaultVolume = 1.0f, float defaultPan = 0.0f, float defaultMinAttenuationDistance = 1.0f, float defaultMaxAttenuationDistance = FLT_MAX);
		~AudioFile();

		irrklang::ISoundSource* GetSource() const { return source; }

		const AudioFileDefaultSettings& GetDefaultSettings() const { return defaultSettings; }
		const float GetLengthMilliSeconds() const { return lengthMilliseconds; }
		const std::string& GetName() const { return name; }
	private:
		irrklang::ISoundSource* source;

		AudioFileDefaultSettings defaultSettings;
		float lengthMilliseconds;
		std::string name;
	};
}