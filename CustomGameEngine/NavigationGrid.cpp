#include "NavigationGrid.h"
#include <glm/gtx/norm.hpp>
namespace Engine {
	NavigationGrid::NavigationGrid()
	{
		nodeSize = 0;
		gridWidth = 0;
		gridHeight = 0;
		straightCost = 0.0f;
		diagonalCost = 0.0f;
	}

	NavigationGrid::NavigationGrid(const std::string& filepath)
	{
		std::ifstream infile(filepath);

		if (infile.is_open()) {
			infile >> nodeSize;
			infile >> gridWidth;
			infile >> gridHeight;
			infile >> straightCost;
			infile >> diagonalCost;

			allNodes.reserve(gridWidth * gridHeight);
			for (int i = 0; i < gridWidth * gridHeight; i++) {
				allNodes.emplace_back(new NavGridNode());
			}

			// Create nodes and their positions
			for (int y = 0; y < gridHeight; y++) {
				for (int x = 0; x < gridWidth; x++) {
					NavGridNode* n = allNodes[(gridWidth * y) + x];
					char type = 0;
					infile >> type;
					n->type = type;
					n->worldPosition = glm::vec3(x * nodeSize, 0.0f, y * nodeSize);
				}
			}

			// Create connections between nodes based on type
			for (int y = 0; y < gridHeight; y++) {
				for (int x = 0; x < gridWidth; x++) {
					NavGridNode* n = allNodes[(gridWidth * y) + x];

					bool spaceAbove = y > 0;
					bool spaceBelow = y < gridHeight - 1;
					bool spaceLeft = x > 0;
					bool spaceRight = x < gridWidth - 1;

					if (spaceAbove) {
						// Get above node
						n->connected[0] = allNodes[(gridWidth * (y - 1)) + x];

						if (spaceRight) {
							// Get upper right node
							n->connected[4] = allNodes[(gridWidth * (y - 1)) + (x + 1)];
						}

						if (spaceLeft) {
							// Get upper left node
							n->connected[7] = allNodes[(gridWidth * (y - 1)) + (x - 1)];
						}
					}

					if (spaceRight) {
						// Get right node
						n->connected[1] = allNodes[(gridWidth * (y)) + (x + 1)];
					}

					if (spaceBelow) {
						// Get below node
						n->connected[2] = allNodes[(gridWidth * (y + 1)) + x];

						if (spaceRight) {
							// Get lower right node
							n->connected[5] = allNodes[(gridWidth * (y + 1)) + (x + 1)];
						}

						if (spaceLeft) {
							// Get lower left node
							n->connected[6] = allNodes[(gridWidth * (y + 1)) + (x - 1)];
						}
					}

					if (spaceLeft) {
						// Get left node
						n->connected[3] = allNodes[(gridWidth * (y)) + (x - 1)];
					}

					for (int i = 0; i < 8; i++) {
						if (n->connected[i]) {
							if (n->connected[i]->type == '.') {
								float cost = straightCost;
								if (i == 6 || i == 5 || i == 4 || i == 7) {
									// diagonal connection
									cost = diagonalCost;
								}
								n->costs[i] = cost;
							}
							else if (n->connected[i]->type == 'x') {
								// not walkable
								n->connected[i] = nullptr;
								n->costs[i] = -1.0f;
							}
						}
					}
				}
			}
		}
		else {
			std::cout << "ERROR::NAVGRID::Failed to load navigation grid at path: " << filepath << std::endl;
			nodeSize = 0;
			gridWidth = 0;
			gridHeight = 0;
			straightCost = 0.0f;
			diagonalCost = 0.0f;
		}
	}

	NavigationGrid::~NavigationGrid()
	{
		for (NavGridNode* node : allNodes) {
			delete node;
		}
	}

	bool NavigationGrid::FindPath(const glm::vec3& start, const glm::vec3& end, NavigationPath& out_path)
	{
		// Find start and end node indices
		
		// If this was just (start.x / nodeSize) then the value will be truncated when cast to int. This is fine for fractional parts less than 0.5
		// However, if the fractional part is above 0.5 then it will still in effect be rounded down. To round up, without actually rounding up, add on half the node size to the initial value before dividing
		// If start.x = 5.4 and nodeSize = 1:
		//		5.4 + 0.5 = 5.9
		//		5.4 should be rounded down anyway, and the result is still less than 6, so truncation will leave us with 5
		// If start.x = 5.6 and nodeSize = 1:
		//		(We want to round up here)
		//		5.6 + 0.5 = 6.1
		//		6.1 when cast to int will be 6, which is in effect the result of rounding up from 5.6
		 
		int startX = (start.x + nodeSize / 2) / nodeSize;
		int startY = (start.z + nodeSize / 2) / nodeSize;

		int endX = (end.x + nodeSize / 2) / nodeSize;
		int endY = (end.z + nodeSize / 2) / nodeSize;

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