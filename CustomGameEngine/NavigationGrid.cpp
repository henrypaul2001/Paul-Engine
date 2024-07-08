#include "NavigationGrid.h"
namespace Engine {
	NavigationGrid::NavigationGrid()
	{

	}

	NavigationGrid::~NavigationGrid()
	{

	}

	bool NavigationGrid::FindPath(const glm::vec3& start, const glm::vec3& end, NavigationPath& out_path)
	{
		return false;
	}

	bool NavigationGrid::NodeInList(NavGridNode* n, std::vector<NavGridNode*>& list) const
	{
		return false;
	}

	NavGridNode* NavigationGrid::RemoveBestNode(std::vector<NavGridNode*>& list) const
	{
		return nullptr;
	}

	float NavigationGrid::Heuristic(NavGridNode* hNode, NavGridNode* endNode) const
	{
		return 0.0f;
	}
}