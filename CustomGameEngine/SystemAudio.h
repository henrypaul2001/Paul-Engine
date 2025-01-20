#pragma once
#include "SystemNew.h"
#include "ComponentTransform.h"
#include "ComponentAudioSource.h"
namespace Engine {
    class SystemAudio : public SystemNew
    {
    public:
        SystemAudio(EntityManagerNew* ecs) : SystemNew(ecs) {}
        ~SystemAudio() {}

        constexpr const char* SystemName() override { return "SYSTEM_AUDIO"; }

        void OnAction(const unsigned int entityID, ComponentTransform& transform, ComponentAudioSource& audio);
        void AfterAction();
    };
}