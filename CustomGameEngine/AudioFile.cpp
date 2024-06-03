#include "AudioFile.h"
namespace Engine {


	AudioFile::AudioFile(irrklang::ISoundSource* source, float defaultVolume, float defaultPan, float defaultMinAttenuationDistance, float defaultMaxAttenuationDistance)
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
		source->drop();
	}
}