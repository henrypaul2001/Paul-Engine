#include "SystemBuildMeshList.h"
namespace Engine {
	std::vector<std::pair<std::pair<glm::vec3, unsigned int>, Mesh*>> SystemBuildMeshList::centrePosAndMeshesList = std::vector<std::pair<std::pair<glm::vec3, unsigned int>, Mesh*>>();
}