#pragma once
#include "System.h"
#include "ComponentTransform.h"
#include "ComponentAudioSource.h"
namespace Engine {
    class SystemAudio : public System
    {
    public:
        SystemAudio(EntityManagerNew* ecs) : System(ecs) {}
        ~SystemAudio() {}

        constexpr const char* SystemName() override { return "SYSTEM_AUDIO"; }

        void OnAction(const unsigned int entityID, ComponentTransform& transform, ComponentAudioSource& audio);
        void AfterAction();
    };
}