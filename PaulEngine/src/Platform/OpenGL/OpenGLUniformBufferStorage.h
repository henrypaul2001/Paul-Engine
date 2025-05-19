#pragma once
#include "PaulEngine/Renderer/UniformBufferStorage.h"

namespace PaulEngine
{
	class OpenGLUniformBufferStorage : public UniformBufferStorage
	{
	public:
		// default usage = GL_DYNAMIC_DRAW
		OpenGLUniformBufferStorage(std::vector<BufferElement> layout, uint32_t binding, unsigned int usage = 0x88E8);
		~OpenGLUniformBufferStorage();

		virtual void UploadStorage() override;
		virtual void UploadStorageForced() override;
		virtual void Bind(uint32_t binding = 0) override;
		virtual void Bind() override;
		virtual uint32_t GetBinding() const override { return m_Binding; }

	private:
		unsigned int m_Usage;
		uint32_t m_Binding;
		uint32_t m_RendererID;
	};
}