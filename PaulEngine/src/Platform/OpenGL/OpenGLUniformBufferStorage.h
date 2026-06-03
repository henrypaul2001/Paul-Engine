#pragma once
#include "PaulEngine/Renderer/Resource/UniformBufferStorage.h"
#include "PaulEngine/Renderer/Resource/Buffer.h"

namespace PaulEngine
{
	class OpenGLUniformBufferStorage : public UniformBufferStorage
	{
	public:
		// default usage = GL_DYNAMIC_DRAW
		OpenGLUniformBufferStorage(std::vector<BufferElement> layout, uint32_t binding, BufferUsage usage = BufferUsage::DYNAMIC_DRAW);
		~OpenGLUniformBufferStorage();

		virtual void UploadStorage() override;
		virtual void UploadStorageForced() override;
		virtual void Bind(uint32_t binding = 0) override;
		virtual void Bind() override;
		virtual uint32_t GetBinding() const override { return m_Binding; }

	private:
		uint32_t m_Binding;
		uint32_t m_RendererID;
		BufferUsage m_Usage;
	};
}