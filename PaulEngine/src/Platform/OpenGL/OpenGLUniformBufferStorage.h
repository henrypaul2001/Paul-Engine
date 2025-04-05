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

		virtual void SetLocalData(const std::string& name, void* data) override;
		virtual void AddDataType(const std::string& name, Ref<ShaderDataTypeStorageBase> data) override;
		virtual void UploadStorage() override;

	private:
		uint32_t m_RendererID;

		struct LayoutElement
		{
			std::string Name;
			Ref<ShaderDataTypeStorageBase> Data;
		};
		std::vector<LayoutElement> m_LayoutStorage;
	};
}