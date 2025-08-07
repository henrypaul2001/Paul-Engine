#pragma once
#include "PaulEngine/Renderer/RenderCommand.h"
#include "PaulEngine/Asset/AssetManager.h"
#include "PaulEngine/Renderer/Asset/Mesh.h"
#include "PaulEngine/Renderer/Asset/Material.h"
#include "PaulEngine/Renderer/RenderPipeline.h"
#include "PaulEngine/Renderer/Resource/UniformBuffer.h"

namespace PaulEngine
{
	namespace RenderTreeUtils
	{
		static Ref<UniformBuffer> s_MeshDataUniformBuffer = nullptr;
#define RENDER_TREE_RESERVE_COUNT 100
	}

	using RenderInput = std::tuple<Ref<VertexArray>, Ref<Material>, glm::mat4, DepthState, FaceCulling, BlendState, int>;

	struct MeshInput
	{
		Ref<Mesh> MeshAsset;
		glm::mat4 Transform;
		int EntityID;
	};

	struct MeshSubmissionData
	{
		glm::mat4 Transform;
		int EntityID;
	};

	struct RenderNode
	{
		uint8_t DataIndex;

		int8_t ParentIndex = -1;
		std::vector<uint8_t> ChildrenIndices = {};
	};

	template <typename T>
	inline static void visitData(const T& data)
	{
		PE_CORE_WARN("Data visited with no specialisation defined");
	}
	template <>
	inline static void visitData<Ref<VertexArray>>(const Ref<VertexArray>& data)
	{
		data->Bind();
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

	struct MeshBin
	{
		MeshBin() { Instances.reserve(RENDER_TREE_RESERVE_COUNT); }
		MeshBin(Ref<VertexArray> vao) { Instances.reserve(RENDER_TREE_RESERVE_COUNT); SharedVertexArray = vao; }
		Ref<VertexArray> SharedVertexArray = nullptr;
		std::vector<MeshSubmissionData> Instances;
		size_t NumInstances() const { return Instances.size(); }
	};

	// TODO: Update this so that mesh bins are not a separate collection and exist in the generic data pools
	// TODO: Add constraint that makes sure one of the Types... is Ref<VertexArray>
	// TODO: Investigate crash when switching to 3DScene.paul
	template <typename... Types>
	class RenderTree
	{
	public:
		RenderTree() : m_DataPools(), m_NumLayers(std::tuple_size<std::tuple<Types...>>::value)
		{
			RenderNode rootNode;
			m_Nodes.push_back(rootNode);
		}
		void Init()
		{
			if (!RenderTreeUtils::s_MeshDataUniformBuffer) {
				RenderTreeUtils::s_MeshDataUniformBuffer = UniformBuffer::Create(sizeof(MeshSubmissionData), 1);
			}
			m_MeshBins.reserve(RENDER_TREE_RESERVE_COUNT);
			forEachDataPool([&](auto& dataPool, std::size_t index) {
				dataPool.reserve(RENDER_TREE_RESERVE_COUNT);
			}, std::make_index_sequence<sizeof...(Types)>());
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
				const uint16_t numChildren = currentNode.ChildrenIndices.size();
				bool success = false;
				for (uint16_t i = 0; i < numChildren; i++)
				{
					const uint16_t childIndex = currentNode.ChildrenIndices[i];
					RenderNode& childNode = m_Nodes[childIndex];

					const auto& childData = dataPool[childNode.DataIndex];

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
					newChild.ParentIndex = currentNodeIndex;

					// Add data to data pool
					newChild.DataIndex = dataPool.size();
					dataPool.push_back(inputData);

					// Add new child to node list
					uint16_t newChildIndex = m_Nodes.size();
					RenderNode& currentNode = m_Nodes[currentNodeIndex];
					currentNode.ChildrenIndices.push_back(newChildIndex);
					m_Nodes.push_back(newChild);

					//currentNode = m_Nodes[newChildIndex];
					currentNodeIndex = newChildIndex;
				}
			}, std::make_index_sequence<sizeof...(Types)>());
		
			// Add to leaf node
			RenderNode& currentNode = m_Nodes[currentNodeIndex];
			if (currentNode.ChildrenIndices.size() == 0)
			{
				currentNode.ChildrenIndices.push_back(m_MeshBins.size());
				m_MeshBins.emplace_back(std::get<Ref<VertexArray>>(input));
			}
			uint16_t meshBinIndex = currentNode.ChildrenIndices[0];
			m_MeshBins[meshBinIndex].Instances.push_back({ std::get<glm::mat4>(input), std::get<int>(input) });
		}

		// Returns the number of draw calls
		uint16_t Flush()
		{
			RenderTreeUtils::s_MeshDataUniformBuffer->Bind(1);

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

		size_t NumLayers() const { return m_NumLayers; }

		static Ref<UniformBuffer> GetMeshSubmissionBuffer() { return RenderTreeUtils::s_MeshDataUniformBuffer; }
		static Ref<UniformBuffer>& GetMeshSubmissionBufferRef() { return RenderTreeUtils::s_MeshDataUniformBuffer; }

	private:
		template <typename Func, std::size_t... I>
		void forEachDataPool(Func&& func, std::index_sequence<I...>)
		{
			// Unroll each index into a function call
			(func(std::get<I>(m_DataPools), I), ...);
		}

		// Used to process individual data from a generic data pool at runtime
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
				ProcessMeshBin(currentNode.ChildrenIndices[0]);
			}
			else
			{
				for (int i = 0; i < currentNode.ChildrenIndices.size(); i++)
				{
					RenderNode childNode = m_Nodes[currentNode.ChildrenIndices[i]];
					uint16_t dataIndex = childNode.DataIndex;

					// Get data from data pools
					// Process data (bind material, set depth test mode, etc)
					forDataPoolElement([&](const auto& data) {
						visitData(data);
					}, currentLayer, dataIndex, std::make_index_sequence<sizeof...(Types)>());

					ProcessNodeChildren(childNode, currentLayer + 1);
				}
			}
		}

		void ProcessMeshBin(const uint16_t binIndex)
		{
			const MeshBin& meshBin = m_MeshBins[binIndex];
			const size_t numInstances = meshBin.NumInstances();
			const Ref<IndexBuffer>& indexBuffer = meshBin.SharedVertexArray->GetIndexBuffer();

			for (size_t i = 0; i < numInstances; i++)
			{
				RenderTreeUtils::s_MeshDataUniformBuffer->SetData(&meshBin.Instances[i], sizeof(MeshSubmissionData));
				RenderCommand::DrawIndexed(meshBin.SharedVertexArray, indexBuffer->GetCount()); // TODO: make sure DrawIndexed() doesn't bind vertex array
				m_DrawCalls++;
			}
		}

		const size_t m_NumLayers;
		std::tuple<std::vector<Types>...> m_DataPools;
		std::vector<RenderNode> m_Nodes;
		std::vector<MeshBin> m_MeshBins;

		uint16_t m_DrawCalls = 0;
	};
}