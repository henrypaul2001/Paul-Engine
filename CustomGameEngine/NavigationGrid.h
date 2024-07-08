#pragma once
#include "NavigationMap.h"
#include <vector>
namespace Engine {

	struct NavGridNode {
		NavGridNode* parent;

		NavGridNode* connected[8];
		float costs[8];

		glm::vec3 worldPosition;

		float f;
		float g;

		int type;
	
		NavGridNode(const glm::vec3& worldPos) {
			for (int i = 0; i < 8; i++) {
				connected[i] = nullptr;
				costs[i] = 0.0f;
			}

			f = 0.0f;
			g = 0.0f;
			type = 0;
			parent = nullptr;
			worldPosition = worldPos;
		}
	};

	class NavigationGrid : public NavigationMap
	{
	public:
		NavigationGrid();
		~NavigationGrid();

		bool FindPath(const glm::vec3& start, const glm::vec3& end, NavigationPath& out_path) override;

	protected:
		bool NodeInList(NavGridNode* n, std::vector<NavGridNode*>& list) const;
		NavGridNode* RemoveBestNode(std::vector<NavGridNode*>& list) const;
		float Heuristic(NavGridNode* hNode, NavGridNode* endNode) const;
		int nodeSize;
		int gridWidth;
		int gridHeight;

		NavGridNode* allNodes[];
	};
}