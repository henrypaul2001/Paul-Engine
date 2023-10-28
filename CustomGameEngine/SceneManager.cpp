#include "SceneManager.h"
namespace Engine
{
	SceneManager::SceneManager(int width, int height, int windowXPos, int windowYPos) {
		this->width = width;
		this->height = height;
		this->windowXPos = windowXPos;
		this->windowYPos = windowYPos;
	}

	SceneManager::~SceneManager() {
		delete& renderer;
		delete& updater;
		delete& keyboardUpDelegate;
		delete& keyboardDownDelegate;
		delete& mouseDelegate;
	}
}