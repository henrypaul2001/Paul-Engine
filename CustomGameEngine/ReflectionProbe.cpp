#include "ReflectionProbe.h"
namespace Engine {
	ReflectionProbe::ReflectionProbe(unsigned int id, const glm::vec3& position)
	{
		this->fileID = id;
		this->worldPosition = position;
	}

	ReflectionProbe::~ReflectionProbe()
	{

	}
}