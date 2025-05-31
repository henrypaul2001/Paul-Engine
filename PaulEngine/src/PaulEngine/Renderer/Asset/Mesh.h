#pragma once
#include "PaulEngine/Asset/Asset.h"
#include "PaulEngine/Renderer/Resource/VertexArray.h"
#include "PaulEngine/Renderer/Resource/Buffer.h"

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/geometric.hpp>

namespace PaulEngine
{
	struct MeshVertex
	{
		glm::vec3 Position = glm::vec3(0.0f);
		glm::vec3 Normal = glm::vec3(0.0f);
		glm::vec2 TexCoords = glm::vec2(0.0f);
		glm::vec3 Tangent = glm::vec3(0.0f);
		glm::vec3 Bitangent = glm::vec3(0.0f);
	};

	static void ComputeTangentsIndexed(MeshVertex* vertices, uint32_t* indices, size_t vertexCount, size_t indexCount);

	struct MeshSpecification
	{
		//const BufferLayout VertexLayout = {
		//	{ ShaderDataType::Float3, "a_Position", false },
		//	{ ShaderDataType::Float3, "a_Normal", true },
		//	{ ShaderDataType::Float2, "a_TexCoords", true },
		//	{ ShaderDataType::Float3, "a_Tangent", true },
		//	{ ShaderDataType::Float3, "a_Bitangent", true }
		//};
		bool CalculateTangents = false;
		BufferUsage UsageType = BufferUsage::STATIC_DRAW;
		DrawPrimitive PrimitiveType = DrawPrimitive::TRIANGLES;
	};

	// As of now, this will only support a fixed vertex buffer layout that all meshes must follow because of how most of the engine shaders use this layout,
	// and, supporting varying layouts would complicate the use of the Vertex struct. However, allowing a raw array of floats instead of Vertex could be a 
	// solution worth looking at to support a different vertex layout
	class Mesh : public Asset
	{
	public:
		Mesh(const MeshSpecification& spec, std::vector<MeshVertex> vertices, std::vector<uint32_t> indices);
		~Mesh() {}

		virtual AssetType GetType() const override { return AssetType::Mesh; }
		
		Ref<VertexArray> GetVertexArray() { return m_VertexArray; }

		uint32_t NumVertices() const { return m_VertexCount; }
		uint32_t NumIndices() const { return m_VertexArray->GetIndexBuffer()->GetCount();; }
	private:
		MeshSpecification m_Spec;
		Ref<VertexArray> m_VertexArray;

		uint32_t m_VertexCount = 0;
	};
}