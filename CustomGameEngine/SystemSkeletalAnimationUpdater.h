#pragma once
#include "SystemNew.h"
#include "ComponentGeometry.h"
#include "ComponentAnimator.h"
namespace Engine {
	class SystemSkeletalAnimationUpdater : public SystemNew
	{
	public:
		SystemSkeletalAnimationUpdater(EntityManagerNew* ecs) : SystemNew(ecs) {}
		~SystemSkeletalAnimationUpdater() {}
		
		constexpr const char* SystemName() override { return "SYSTEM_SKELTAL_ANIM_UPDATE"; }

		void OnAction(const unsigned int entityID, ComponentGeometry& geometry, ComponentAnimator& animator);
		void AfterAction();
	};
}