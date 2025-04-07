#pragma once
#include "PaulEngine/Renderer/UniformBufferStorage.h"

namespace PaulEngine
{
	class OpenGLUniformBufferStorage : public UniformBufferStorage
	{
	public:
		// default usage = GL_DYNAMIC_DRAW
		OpenGLUniformBufferStorage(size_t size, uint32_t binding, unsigned int usage = 0x88E8);
		~OpenGLUniformBufferStorage();

		virtual uint32_t GetBinding() const override;
		virtual void SetLocalData(const std::string& name, void* data) override;
		virtual void AddDataType(const std::string& name, Ref<ShaderDataTypeStorageBase> data) override;
		virtual void UploadStorage() override;

		virtual std::vector<LayoutElement>& GetLayoutStorage() override { return m_LayoutStorage; }
		virtual const std::vector<LayoutElement>& GetLayoutStorage() const override { return m_LayoutStorage; }

	private:
		uint32_t m_Binding;
		uint32_t m_RendererID;

		std::vector<LayoutElement> m_LayoutStorage;
	};
}