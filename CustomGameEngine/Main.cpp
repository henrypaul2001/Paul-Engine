#include "GameSceneManager.h"
//#include "EntityManagerNew.h"
int main()
{
	Engine::GameSceneManager game = Engine::GameSceneManager(2560, 1440, 200, 80);
	game.Run();
	//Engine::EntityManagerNew ecs;

	return 0;
}