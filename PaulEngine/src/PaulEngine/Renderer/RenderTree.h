#pragma once
#include "PaulEngine/Asset/AssetManager.h"
#include "PaulEngine/Renderer/Asset/Mesh.h"
#include "PaulEngine/Renderer/Asset/Material.h"
#include "PaulEngine/Renderer/RenderPipeline.h"

namespace PaulEngine
{
	//struct RenderInput
	//{
	//	const AssetHandle MeshHandle;
	//	const AssetHandle MaterialHandle;
	//	const glm::mat4 Transform;
	//	const DepthState DepthState;
	//	const FaceCulling CullState;
	//	const BlendState BlendState;
	//	const int EntityID;
	//};

	using RenderInput = std::tuple<Ref<Mesh>, Ref<Material>, glm::mat4, DepthState, FaceCulling, BlendState, int>();

	class RenderNode
	{
	public:


		uint8_t m_DataIndex;

		int8_t m_ParentIndex = -1;
		std::vector<uint8_t> m_ChildrenIndices;
	};

	template <typename... Types>
	class RenderTree
	{
	public:
		RenderTree() : m_DataPools()
		{
			RenderNode rootNode;
			m_Nodes.push_back(rootNode);

			m_NumLayers = std::tuple_size<std::tuple<Types...>>::value;
		}

		void PushMesh(RenderInput input)
		{
			RenderNode& currentNode = m_RootNode;
			uint8_t layerIndex = 0;

			while (layerIndex < m_NumLayers)
			{
				// Test child nodes
				bool success = false;
				for (int i = 0; i < currentNode.m_ChildrenIndices.size(); i++)
				{
					RenderNode& childNode = m_Nodes[currentNode.m_ChildrenIndices[i]];

					// Get data from node and compare with input
					const auto& nodeData = std::get<layerIndex>(m_DataPools)[currentNode.m_DataIndex];
					const auto& inputData = std::get<std::tuple_element<layerIndex, std::tuple<Types...>>>(input);

					if (nodeData == inputData)
					{
						// Mesh belongs in this node, test children
						currentNode = childNode;
						success = true;
						break;
					}

					// continue testing siblings
				}

				if (!success)
				{
					// Mesh doesn't belong to any existing branches, create new branch
					break;
				}

				layerIndex++;
			}

			// Add to mesh bin

		}


	//private:
		size_t m_NumLayers;
		std::tuple<std::vector<Types>...> m_DataPools;
		
		RenderNode m_RootNode;
		std::vector<RenderNode> m_Nodes;
	};

	static void Test()
	{
		RenderTree<Ref<Material>, DepthState> m_Test;
		auto testCollection = std::get<1>(m_Test.m_DataPools);
	}
}