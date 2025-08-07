#pragma once
#include "PaulEngine/Renderer/Resource/Buffer.h"

namespace PaulEngine {
	enum class DrawPrimitive
	{
		None = 0,
		POINTS,
		LINES, LINE_LOOP, LINE_STRIP, LINES_ADJACENCY, LINE_STRIP_ADJACENCY,
		TRIANGLES, TRIANGLE_STRIP, TRIANGLE_FAN, TRIANGLES_ADJACENCY, TRIANGLE_STRIP_ADJACENCY
	};

	class VertexArray
	{
	public:
		virtual ~VertexArray();

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) = 0;
		virtual void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer) = 0;

		virtual const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const = 0;
		virtual const Ref<IndexBuffer>& GetIndexBuffer() const = 0;
		virtual DrawPrimitive GetDrawPrimitive() const = 0;

		static Ref<VertexArray> Create(DrawPrimitive drawPrimitive = DrawPrimitive::TRIANGLES);

		virtual bool operator ==(const VertexArray* other) const = 0;
		virtual bool operator !=(const VertexArray* other) const = 0;
	};
	inline bool operator==(const Ref<VertexArray>& left, const Ref<VertexArray>& right)
	{
		return (left.get() == right.get());
	}
}