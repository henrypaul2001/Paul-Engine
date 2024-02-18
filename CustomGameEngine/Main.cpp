#include "GameSceneManager.h"

int main()
{
	Engine::GameSceneManager game = Engine::GameSceneManager(1920, 1080, 200, 80);
	game.Run();

	return 0;
}