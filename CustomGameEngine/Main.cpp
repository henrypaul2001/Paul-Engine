#include "GameSceneManager.h"

int main()
{
	Engine::GameSceneManager game = Engine::GameSceneManager(3840, 2160, 200, 80); // 2560 1440
	game.Run();

	return 0;
}