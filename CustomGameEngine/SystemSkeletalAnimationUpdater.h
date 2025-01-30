#pragma once
#include "System.h"
#include "ComponentGeometry.h"
#include "ComponentAnimator.h"
namespace Engine {
	class SystemSkeletalAnimationUpdater : public System
	{
	public:
		SystemSkeletalAnimationUpdater(EntityManager* ecs) : System(ecs) {}
		~SystemSkeletalAnimationUpdater() {}
		
		constexpr const char* SystemName() override { return "SYSTEM_SKELTAL_ANIM_UPDATE"; }

		void OnAction(const unsigned int entityID, ComponentGeometry& geometry, ComponentAnimator& animator);
		void AfterAction();
	};
}