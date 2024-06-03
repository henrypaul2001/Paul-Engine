#include "AudioManager.h"
#include <iostream>
namespace Engine {
	AudioManager* AudioManager::instance = nullptr;
	AudioManager::AudioManager() {
		soundEngine = irrklang::createIrrKlangDevice();
		if (!soundEngine) {
			std::cout << "ERROR::AUDIOMANAGER::Error initialising sound engine" << std::endl;
			return;
		}
	}

	AudioManager::~AudioManager()
	{
		soundEngine->drop();

		delete instance;
	}

	AudioManager* AudioManager::GetInstance()
	{
		if (instance == nullptr) {
			instance = new AudioManager();
		}
		return instance;
	}
}