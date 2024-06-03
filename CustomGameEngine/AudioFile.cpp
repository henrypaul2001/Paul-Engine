#include "AudioFile.h"
namespace Engine {
	AudioFile::AudioFile(irrklang::ISoundSource* source, AudioFileDefaultSettings defaultSettings)
	{
		this->source = source;
		this->defaultSettings = defaultSettings;

		name = source->getName();
		lengthMilliseconds = source->getPlayLength();
	}

	AudioFile::AudioFile(irrklang::ISoundSource* source)
	{
		this->source = source;

		this->defaultSettings.defaultMaxAttenuationDistance = FLT_MAX;
		this->defaultSettings.defaultMinAttenuationDistance = 1.0f;
		this->defaultSettings.defaultVolume = 1.0f;
		this->defaultSettings.defaultPan = 0.0f;

		name = source->getName();
		lengthMilliseconds = source->getPlayLength();
	}

	AudioFile::~AudioFile()
	{

	}
}