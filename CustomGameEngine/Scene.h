#pragma once
#include "SceneManager.h"
namespace Engine 
{
	class Scene
	{
	protected:
		SceneManager* sceneManager;
	public:
		static float dt;
		Scene(SceneManager* sceneManager);
		~Scene();

		virtual void Render() = 0;
		virtual void Update() = 0;
		virtual void Close() = 0;
	};
}

