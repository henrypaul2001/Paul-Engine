#pragma once
#include "Component.h"
#include <glm/ext/vector_float3.hpp>
namespace Engine {
	enum LightTypes {
		DIRECTIONAL,
		POINT,
		SPOT
	};

	class ComponentLight : public Component
	{
	private:
		LightTypes type;

		void DefaultDirectional();
		void DefaultSpot();
		void DefaultPoint();
	public:
		ComponentLight(const ComponentLight& old_component);
		ComponentLight(LightTypes type);
		~ComponentLight();

		LightTypes GetLightType() { return type; }

		ComponentTypes ComponentType() override { return COMPONENT_LIGHT; }
		void Close() override;

		Component* Copy() override { return new ComponentLight(*this); }

		// Universal
		glm::vec3 Colour;
		glm::vec3 Ambient;
		glm::vec3 Specular;

		bool CastShadows;
		bool Active;

		float MinShadowBias;
		float MaxShadowBias;
		float Near;
		float Far;

		// Spot + Point
		float Linear;
		float Quadratic;
		float Constant;

		// Spot
		float Cutoff;
		float OuterCutoff;

		// Directional + Spot
		glm::vec3 Direction;

		// Spot
		glm::vec3 WorldDirection;

		// Directional
		float ShadowProjectionSize;
		float DirectionalLightDistance;
	};
}