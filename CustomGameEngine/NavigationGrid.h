#pragma once
#include "NavigationMap.h"
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
namespace Engine {

	struct NavGridNode {
		NavGridNode* parent;

		NavGridNode* connected[8];
		float costs[8];

		glm::vec3 worldPosition;

		float f;
		float g;

		int type;
	
		NavGridNode() {
			for (int i = 0; i < 8; i++) {
				connected[i] = nullptr;
				costs[i] = 0.0f;
			}

			f = 0.0f;
			g = 0.0f;
			type = 0;
			parent = nullptr;
			worldPosition = glm::vec3();
		}
	};

	class NavigationGrid : public NavigationMap
	{
	public:
		NavigationGrid();
		NavigationGrid(const std::string& filepath);
		~NavigationGrid();

		bool FindPath(const glm::vec3& start, const glm::vec3& end, NavigationPath& out_path) override;

		const int GetGridWidth() const { return gridWidth; }
		const int GetGridHeight() const { return gridHeight; }
		const float GetNodeSize() const { return nodeSize; }
		const std::vector<NavGridNode*>& GetNodes() const { return allNodes; }

	protected:
		bool NodeInList(NavGridNode* n, std::vector<NavGridNode*>& list) const;
		NavGridNode* RemoveBestNode(std::vector<NavGridNode*>& list) const;
		float Heuristic(NavGridNode* hNode, NavGridNode* endNode) const;

		float nodeSize;
		int gridWidth;
		int gridHeight;

		float straightCost;
		float diagonalCost;

		std::vector<NavGridNode*> allNodes;
	};
}