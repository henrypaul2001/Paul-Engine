#include "AudioFile.h"
namespace Engine {


	AudioFile::AudioFile(irrklang::ISoundSource* source, float defaultVolume = 1.0f, float defaultPan = 0.0f, float defaultMinAttenuationDistance = 1.0f, float defaultMaxAttenuationDistance = FLT_MAX)
	{
		this->source = source;

		this->defaultSettings.defaultMaxAttenuationDistance = defaultMaxAttenuationDistance;
		this->defaultSettings.defaultMinAttenuationDistance = defaultMinAttenuationDistance;
		this->defaultSettings.defaultVolume = defaultVolume;
		this->defaultSettings.defaultPan = defaultPan;

		name = source->getName();
		lengthMilliseconds = source->getPlayLength();
	}

	AudioFile::~AudioFile()
	{

	}
}