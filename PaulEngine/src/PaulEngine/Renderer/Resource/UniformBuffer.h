#pragma once
#include "PaulEngine/Core/Core.h"
#include "PaulEngine/Renderer/Resource/Buffer.h"
namespace PaulEngine
{
	class UniformBuffer
	{
	public:
		virtual ~UniformBuffer() {}

		virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) = 0;
		virtual void Bind(uint32_t binding) = 0;

		static Ref<UniformBuffer> Create(uint32_t size, uint32_t binding, BufferUsage usage = BufferUsage::DYNAMIC_DRAW);
	};
}