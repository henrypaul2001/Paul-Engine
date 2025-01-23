#pragma once
#include <glm/ext/vector_float3.hpp>
namespace Engine {

	// todo: again, this will be looked at properly with the renderer rewrite

	enum LightTypes {
		DIRECTIONAL,
		POINT,
		SPOT
	};

	class ComponentLight
	{
	public:
		ComponentLight(LightTypes type);
		~ComponentLight();

		LightTypes GetLightType() const { return type; }

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

	private:
		LightTypes type;

		void DefaultDirectional();
		void DefaultSpot();
		void DefaultPoint();
	};
}