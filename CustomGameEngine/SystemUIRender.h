#pragma once
#include "ComponentTransform.h"
#include "ComponentUICanvas.h"
namespace Engine {
	class SystemUIRender
	{
	public:
		SystemUIRender() {}
		~SystemUIRender() {}

		void OnAction(const unsigned int entityID, ComponentTransform& transform, ComponentUICanvas& canvas);
	};
}