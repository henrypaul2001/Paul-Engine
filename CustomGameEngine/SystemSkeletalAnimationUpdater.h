#pragma once
#include "System.h"
#include "ComponentGeometry.h"
#include "ComponentTransform.h"
#include "ComponentAnimator.h"
namespace Engine {
	class SystemSkeletalAnimationUpdater : public System
	{
	public:
		SystemSkeletalAnimationUpdater();
		~SystemSkeletalAnimationUpdater();

		SystemTypes Name() override { return SYSTEM_SKELETAL_ANIM_UPDATE; }
		void OnAction(Entity* entity) override;
		void AfterAction() override;

	private:
		const ComponentTypes MASK = (COMPONENT_GEOMETRY | COMPONENT_ANIMATOR);

		void UpdateAnimator(ComponentGeometry* geometry, ComponentAnimator* animator);
	};
}