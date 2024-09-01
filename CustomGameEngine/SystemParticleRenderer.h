#pragma once
#include "System.h"
#include "ComponentTransform.h"
#include "ComponentParticleGenerator.h"
namespace Engine {
	class SystemParticleRenderer : public System
	{
    public:
        SystemParticleRenderer();
        ~SystemParticleRenderer();

        SystemTypes Name() override { return SYSTEM_PARTICLE_RENDER; }
        void Run(const std::vector<Entity*>& entityList) override;
        void OnAction(Entity* entity) override;
        void AfterAction() override;

    private:
        const ComponentTypes MASK = (COMPONENT_TRANSFORM | COMPONENT_PARTICLE_GENERATOR);

        void Draw(ComponentTransform* transform, ComponentParticleGenerator* generator);
	};
}