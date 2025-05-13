#include "pepch.h"
#include "OpenGLUniformBufferStorage.h"

#include <glad/glad.h>

namespace PaulEngine
{
	OpenGLUniformBufferStorage::OpenGLUniformBufferStorage(size_t size, uint32_t binding, unsigned int usage) : m_Binding(binding)
	{
		glCreateBuffers(1, &m_RendererID);
		glNamedBufferData(m_RendererID, size, nullptr, (GLenum)usage);
		glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_RendererID);
	}

	OpenGLUniformBufferStorage::~OpenGLUniformBufferStorage()
	{
		glDeleteBuffers(1, &m_RendererID);
	}

	uint32_t OpenGLUniformBufferStorage::GetBinding() const
	{
		return m_Binding;
	}

	Ref<ShaderDataTypeStorageBase> OpenGLUniformBufferStorage::GetLocalData(const std::string& name)
	{
		for (const LayoutElement& e : m_LayoutStorage) {
			if (e.Name == name) {
				return e.Data;
			}
		}
		PE_CORE_ERROR("Name '{0}' not found in uniform buffer data layout", name);
		return nullptr;
	}

	void OpenGLUniformBufferStorage::SetLocalData(const std::string& name, void* data)
	{
		PE_PROFILE_FUNCTION();
		ShaderDataTypeStorageBase* base = GetLocalData(name).get();
		if (base)
		{
			base->SetData(data);
		}
	}

	void OpenGLUniformBufferStorage::AddDataType(const std::string& name, Ref<ShaderDataTypeStorageBase> data)
	{
		PE_PROFILE_FUNCTION();
		for (const LayoutElement& e : m_LayoutStorage) {
			if (e.Name == name) {
				PE_CORE_ASSERT(false, "Cannot add duplicate names to layout storage");
				return;
			}
		}

		m_LayoutStorage.push_back({ name, data });
	}

	void OpenGLUniformBufferStorage::UploadStorage()
	{
		PE_PROFILE_FUNCTION();
		size_t offset = 0;
		for (const LayoutElement& e : m_LayoutStorage) {
			glNamedBufferSubData(m_RendererID, offset, e.Data->Size(), e.Data->GetData());
			offset += e.Data->Size();
		}
	}

	void OpenGLUniformBufferStorage::Bind(uint32_t binding)
	{
		m_Binding = binding;
		glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_RendererID);
	}
}