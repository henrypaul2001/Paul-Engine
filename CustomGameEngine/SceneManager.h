#pragma once
#include <functional>
namespace Engine
{
	using SceneDelegate = std::function<void()>;

	class SceneManager
	{
		public:
			SceneDelegate renderer;
			SceneDelegate updater;
	};
}

