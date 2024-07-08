#include "NavigationGrid.h"
#include <glm/gtx/norm.hpp>
namespace Engine {
	NavigationGrid::NavigationGrid()
	{

	}

	NavigationGrid::~NavigationGrid()
	{

	}

	bool NavigationGrid::FindPath(const glm::vec3& start, const glm::vec3& end, NavigationPath& out_path)
	{
		// find start and end node indices
		int startX = (start.x / nodeSize);
		int startY = (start.z / nodeSize);

		int endX = (end.x / nodeSize);
		int endY = (end.z / nodeSize);

		// Check if inside grid region
		if (startX < 0 || startX > gridWidth - 1 || startY < 0 || startY > gridHeight - 1) {
			return false;
		}
		if (endX < 0 || endX > gridWidth - 1 || endY < 0 || endY > gridHeight - 1) {
			return false;
		}

		NavGridNode* startNode = allNodes[(startY * gridWidth) + startX];
		NavGridNode* endNode = allNodes[(endY * gridWidth) + endX];

		std::vector<NavGridNode*> openList;
		std::vector<NavGridNode*> closedList;

		openList.emplace_back(startNode);

		startNode->f = 0.0f;
		startNode->g = 0.0f;
		startNode->parent = nullptr;

		NavGridNode* currentBestNode = nullptr;

		while (!openList.empty()) {
			currentBestNode = RemoveBestNode(openList);

			if (currentBestNode == endNode) {
				// Path found
				// Backtrack to create waypoints

				NavGridNode* node = endNode;

				while (node != nullptr) {
					out_path.AddWaypoint(node->worldPosition);
					node = node->parent;
				}

				return true;
			}
			else {
				// Check neighbours
				for (int i = 0; i < 8; i++) {
					NavGridNode* neighbour = currentBestNode->connected[i];
					if (!neighbour) {
						continue;
					}

					if (NodeInList(neighbour, closedList)) {
						continue;
					}

					float h = Heuristic(neighbour, endNode);
					float g = currentBestNode->g + currentBestNode->costs[i];
					float f = h + g;

					bool inOpen = NodeInList(neighbour, openList);

					if (!inOpen) {
						// First time this neighbour has been visited
						openList.emplace_back(neighbour);
					}

					if (!inOpen || f < neighbour->f) {
						neighbour->parent = currentBestNode;
						neighbour->f = f;
						neighbour->g = g;
					}
				}
				closedList.emplace_back(currentBestNode);
			}
		}
		return false; // no path found
	}

	bool NavigationGrid::NodeInList(NavGridNode* n, std::vector<NavGridNode*>& list) const
	{
		std::vector<NavGridNode*>::iterator i = std::find(list.begin(), list.end(), n);
		return i != list.end();
	}

	// Find the node with the best 'f' score and remove it, best node is returned
	NavGridNode* NavigationGrid::RemoveBestNode(std::vector<NavGridNode*>& list) const
	{
		std::vector<NavGridNode*>::iterator bestI = list.begin();

		NavGridNode* bestNode = list[0];

		for (auto i = list.begin(); i != list.end(); i++) {
			if ((*i)->f < bestNode->f) {
				bestNode = (*i);
				bestI = i;
			}
		}

		list.erase(bestI);
		return bestNode;
	}

	float NavigationGrid::Heuristic(NavGridNode* hNode, NavGridNode* endNode) const
	{
		// get distance without expensive sqr root
		return glm::distance2(hNode->worldPosition, endNode->worldPosition);
	}
}