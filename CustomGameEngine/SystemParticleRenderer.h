#pragma once
#include "ComponentTransform.h"
#include "ComponentParticleGenerator.h"
namespace Engine {
	class SystemParticleRenderer
	{
    public:
        SystemParticleRenderer() {}
        ~SystemParticleRenderer() {}

        void OnAction(const unsigned int entityID, ComponentTransform& transform, ComponentParticleGenerator& generator);
	};
}