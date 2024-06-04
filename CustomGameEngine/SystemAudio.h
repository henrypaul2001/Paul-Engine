#pragma once
#include "System.h"
#include "ComponentTransform.h"
#include "ComponentAudioSource.h"
namespace Engine {
    class SystemAudio : public System
    {
    public:
        SystemAudio();
        ~SystemAudio();

        SystemTypes Name() override { return SYSTEM_AUDIO; }
        void OnAction(Entity* entity) override;
        void AfterAction() override;

    private:
        const ComponentTypes MASK = (COMPONENT_TRANSFORM | COMPONENT_AUDIO_SOURCE);
        void UpdateAudio(ComponentTransform* transform, ComponentAudioSource* audio);
    };
}