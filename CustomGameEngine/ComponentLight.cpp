#include "ComponentLight.h"
#include <glm/trigonometric.hpp>
#include "LightManager.h"
namespace Engine {
	ComponentLight::ComponentLight(LightTypes type)
	{
		this->type = type;
		switch (type) {
			case DIRECTIONAL:
				DefaultDirectional();
			case SPOT:
				DefaultSpot();
			case POINT:
				DefaultPoint();
		}
	}

	ComponentLight::~ComponentLight()
	{

	}

	void ComponentLight::DefaultDirectional()
	{
		Direction = glm::vec3(0.0f, -0.5f, -1.0f);

		Colour = glm::vec3(1.0f, 1.0f, 1.0f);
		Specular = glm::vec3(1.0f, 1.0f, 1.0f);
		Ambient = glm::vec3(0.2f, 0.2f, 0.2f);
	}

	void ComponentLight::DefaultSpot()
	{
		Colour = glm::vec3(1.0f, 1.0f, 1.0f);
		Specular = glm::vec3(1.0f, 1.0f, 1.0f);
		Ambient = glm::vec3(0.2f, 0.2f, 0.2f);

		Constant = 1.0f;
		Linear = 0.09f;
		Quadratic = 0.032f;

		Direction = glm::vec3(0.0f, 0.0f, -1.0f);
		Cutoff = glm::cos(glm::radians(12.5f));
		OuterCutoff = glm::cos(glm::radians(17.5f));
	}

	void ComponentLight::DefaultPoint()
	{
		Colour = glm::vec3(1.0f, 1.0f, 1.0f);
		Specular = glm::vec3(1.0f, 1.0f, 1.0f);
		Ambient = glm::vec3(0.2f, 0.2f, 0.2f);

		Constant = 1.0f;
		Linear = 0.09f;
		Quadratic = 0.032f;
	}

	void ComponentLight::Close()
	{

	}
}