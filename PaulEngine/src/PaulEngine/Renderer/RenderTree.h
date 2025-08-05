#pragma once
#include "PaulEngine/Renderer/RenderCommand.h"
#include "PaulEngine/Asset/AssetManager.h"
#include "PaulEngine/Renderer/Asset/Mesh.h"
#include "PaulEngine/Renderer/Asset/Material.h"
#include "PaulEngine/Renderer/RenderPipeline.h"
#include "PaulEngine/Renderer/Resource/UniformBuffer.h"

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

	template <typename T>
	inline static void visitData(const T& data)
	{
		PE_CORE_WARN("Data visited with no specialisation defined");
	}

	template <>
	inline static void visitData<Ref<Material>>(const Ref<Material>& data)
	{
		data->Bind();
	}

	template <>
	inline static void visitData<DepthState>(const DepthState& data)
	{
		if (data.Test) { RenderCommand::EnableDepth(); }
		else { RenderCommand::DisableDepth(); }
		RenderCommand::DepthMask(data.Write);
		RenderCommand::DepthFunc(data.Func);
	}

	template <>
	inline static void visitData<FaceCulling>(const FaceCulling& data)
	{
		RenderCommand::SetFaceCulling(data);
	}

	template <>
	inline static void visitData<BlendState>(const BlendState& data)
	{
		if (data.Enabled) { RenderCommand::EnableBlend(); }
		else { RenderCommand::DisableBlend(); }
		RenderCommand::BlendFunc(data.SrcFactor, data.DstFactor);
		RenderCommand::BlendColour(data.ConstantColour);
		RenderCommand::BlendEquation(data.Equation);
	}

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
			uint16_t currentNodeIndex = 0;
		
			forEachDataPool([&](auto& dataPool, std::size_t index) {
				// Do stuff
				using DataType = typename std::decay_t<decltype(dataPool)>::value_type;
				const auto& inputData = std::get<DataType>(input);

				RenderNode& currentNode = m_Nodes[currentNodeIndex];

				// Test child nodes against input data to find matching branch
				const uint16_t numChildren = currentNode.m_ChildrenIndices.size();
				bool success = false;
				for (uint16_t i = 0; i < numChildren; i++)
				{
					const uint16_t childIndex = currentNode.m_ChildrenIndices[i];
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
					uint16_t newChildIndex = m_Nodes.size();
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
			uint16_t meshBinIndex = currentNode.m_ChildrenIndices[0];
			m_MeshBins[meshBinIndex].push_back({ std::get<Ref<Mesh>>(input), std::get<glm::mat4>(input), std::get<int>(input) });
		}

		// Returns the number of draw calls
		uint16_t Flush()
		{
			uint16_t meshBinsVisited = 0;
			uint16_t currentNodeIndex = 0;
			
			// Traverse from root node to first mesh bin and execute each node on the path
			ProcessNodeChildren(m_Nodes[0], 0);

			uint16_t totalDrawCalls = m_DrawCalls;

			// Clear structure
			Reset();

			return totalDrawCalls;
		}

		void Reset()
		{
			m_Nodes.clear();
			RenderNode rootNode;
			m_Nodes.push_back(rootNode);

			forEachDataPool([](auto& dataPool, std::size_t index) {
				dataPool.clear();
				}, std::make_index_sequence<sizeof...(Types)>());

			m_MeshBins.clear();

			m_DrawCalls = 0;
		}

		uint16_t MeshBinCount() const { return m_MeshBins.size(); }

	//private:
		const size_t m_NumLayers;
		std::tuple<std::vector<Types>...> m_DataPools;

		std::vector<RenderNode> m_Nodes;
		
		std::vector<std::vector<MeshInput>> m_MeshBins;

		uint16_t m_DrawCalls = 0;
		Ref<UniformBuffer> m_MeshDataUniformBuffer;

	private:
		template <typename Func, std::size_t... I>
		void forEachDataPool(Func&& func, std::index_sequence<I...>)
		{
			// Unroll each index into a function call
			(func(std::get<I>(m_DataPools), I), ...);
		}

		template <typename Func, std::size_t... I>
		void forDataPoolElement(Func&& func, uint16_t dataPoolIndex, uint16_t elementIndex, std::index_sequence<I...>)
		{
			auto verifyFunc = [&](auto& dataPool, std::size_t actualIndex) {
				if (actualIndex == dataPoolIndex)
				{
					func(dataPool[elementIndex]);
				}
			};

			(verifyFunc(std::get<I>(m_DataPools), I), ...);
		}


		void ProcessNodeChildren(RenderNode currentNode, uint16_t currentLayer)
		{
			if (currentLayer == m_NumLayers)
			{
				ProcessMeshBin(currentNode.m_ChildrenIndices[0]);
			}
			else
			{
				for (int i = 0; i < currentNode.m_ChildrenIndices.size(); i++)
				{
					RenderNode childNode = m_Nodes[currentNode.m_ChildrenIndices[i]];
					uint16_t dataIndex = childNode.m_DataIndex;

					// Get data from data pools
					// Process data (bind material, set depth test mode, etc)
					forDataPoolElement([&](const auto& data) {
						visitData(data);
					}, currentLayer, dataIndex, std::make_index_sequence<sizeof...(Types)>());

					ProcessNodeChildren(childNode, currentLayer + 1);
				}
			}
		}

		struct MeshSubmissionData
		{
			glm::mat4 Transform;
			int EntityID;
		};
		MeshSubmissionData MeshDataBuffer;
		struct MeshInput
		{
			Ref<Mesh> MeshAsset;
			glm::mat4 Transform;
			int EntityID;
		};
		void ProcessMeshBin(const uint16_t binIndex)
		{
			const auto& meshBin = m_MeshBins[binIndex];

			for (const auto& meshInput : meshBin)
			{
				MeshSubmissionData bufferData;
				bufferData.Transform = meshInput.Transform;
				bufferData.EntityID = meshInput.EntityID;
				m_MeshDataUniformBuffer->SetData(&bufferData, sizeof(MeshSubmissionData));
			
				Ref<VertexArray> vertexArray = meshInput.MeshAsset->GetVertexArray();
				RenderCommand::DrawIndexed(vertexArray, vertexArray->GetIndexBuffer()->GetCount());
				m_DrawCalls++;
			}
			//PE_CORE_TRACE("Mesh bin visited");
		}
	};

	static void Test(Ref<UniformBuffer> meshDataBuffer)
	{
		RenderTree<Ref<Material>, DepthState, FaceCulling, BlendState> m_Test;
		m_Test.m_MeshDataUniformBuffer = meshDataBuffer;
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

		m_Test.Flush();
	}
}