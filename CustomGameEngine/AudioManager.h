#pragma once
#include "irrklang/irrklang.h"
namespace Engine {
	class AudioManager
	{
	public:
		AudioManager(AudioManager& other) = delete; // singleton should not be cloneable
		void operator=(const AudioManager&) = delete; // singleton should not be assignable

		~AudioManager();

		static AudioManager* GetInstance();

	private:
		AudioManager();
		static AudioManager* instance;

		irrklang::ISoundEngine* soundEngine;
	};
}