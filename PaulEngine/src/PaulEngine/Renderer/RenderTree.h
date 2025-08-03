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

	using RenderInput = std::tuple<Ref<Mesh>, Ref<Material>, glm::mat4, DepthState, FaceCulling, BlendState, int>;

	struct MeshInput
	{
		Ref<Mesh> MeshAsset;
		glm::mat4 Transform;
		int EntityID;
	};

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
		RenderTree() : m_DataPools(), m_NumLayers(std::tuple_size<std::tuple<Types...>>::value)
		{
			RenderNode rootNode;
			m_Nodes.push_back(rootNode);
		}
		
		void PushMesh(RenderInput input)
		{
			//RenderNode& currentNode = m_Nodes[0]; // m_Nodes[0] == rootNode
			uint8_t currentNodeIndex = 0;
		
			forEachDataPool([&, this, input](auto& dataPool, std::size_t index) {
				// Do stuff
				using DataType = typename std::decay_t<decltype(dataPool)>::value_type;
				const auto& inputData = std::get<DataType>(input);

				RenderNode& currentNode = m_Nodes[currentNodeIndex];

				// Test child nodes against input data to find matching branch
				const uint8_t numChildren = currentNode.m_ChildrenIndices.size();
				bool success = false;
				for (uint8_t i = 0; i < numChildren; i++)
				{
					const uint8_t childIndex = currentNode.m_ChildrenIndices[i];
					RenderNode& childNode = m_Nodes[childIndex];

					const auto& childData = dataPool[childNode.m_DataIndex];

					if (childData == inputData)
					{
						success = true;

						//currentNode = childNode;
						currentNodeIndex = childIndex;

						break;
					}
				}

				if (!success)
				{
					// No matching branch found, create new branch

					// Create child
					RenderNode newChild;
					newChild.m_ParentIndex = currentNodeIndex;

					// Add data to data pool
					newChild.m_DataIndex = dataPool.size();
					dataPool.push_back(inputData);

					// Add new child to node list
					uint8_t newChildIndex = m_Nodes.size();
					RenderNode& currentNode = m_Nodes[currentNodeIndex];
					currentNode.m_ChildrenIndices.push_back(newChildIndex);
					m_Nodes.push_back(newChild);

					//currentNode = m_Nodes[newChildIndex];
					currentNodeIndex = newChildIndex;
				}
			}, std::make_index_sequence<sizeof...(Types)>());
		
			// Add to leaf node
			RenderNode& currentNode = m_Nodes[currentNodeIndex];
			if (currentNode.m_ChildrenIndices.size() == 0)
			{
				currentNode.m_ChildrenIndices.push_back(m_MeshBins.size());
				m_MeshBins.push_back({});
			}
			uint8_t meshBinIndex = currentNode.m_ChildrenIndices[0];
			m_MeshBins[meshBinIndex].push_back({ std::get<Ref<Mesh>>(input), std::get<glm::mat4>(input), std::get<int>(input) });
		}

	//private:
		const size_t m_NumLayers;
		std::tuple<std::vector<Types>...> m_DataPools;

		std::vector<RenderNode> m_Nodes;
		
		std::vector<std::vector<MeshInput>> m_MeshBins;

	private:
		template <typename Func, std::size_t... I>
		void forEachDataPool(Func&& func, std::index_sequence<I...>)
		{
			// Unroll each index into a function call
			(func(std::get<I>(m_DataPools), I), ...);
		}
	};

	static void Test()
	{
		RenderTree<Ref<Material>, DepthState> m_Test;
		auto testCollection = std::get<0>(m_Test.m_DataPools);

		RenderInput testInput1 = { nullptr, nullptr, glm::mat4(1.0f), {DepthFunc::GREATER, false, true}, FaceCulling::BACK, BlendState(), -1};
		RenderInput testInput2 = { nullptr, nullptr, glm::mat4(1.0f), {DepthFunc::LESS, false, true}, FaceCulling::BACK, BlendState(), -1 };
		RenderInput testInput3 = { nullptr, nullptr, glm::mat4(1.0f), {DepthFunc::GREATER, false, false}, FaceCulling::BACK, BlendState(), -1 };
		m_Test.PushMesh(testInput1);
		m_Test.PushMesh(testInput1);
		m_Test.PushMesh(testInput1);
		m_Test.PushMesh(testInput2);
		m_Test.PushMesh(testInput2);
		m_Test.PushMesh(testInput3);
		m_Test.PushMesh(testInput2);
		m_Test.PushMesh(testInput2);
		m_Test.PushMesh(testInput1);
	}
}