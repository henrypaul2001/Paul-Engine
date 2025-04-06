#pragma once
#include "PaulEngine/Core/Core.h"
#include "UniformBufferStorage.h"

namespace PaulEngine
{
	enum class ShaderParameterType
	{
		None = 0,
		UBO,
		Sampler2D,
		Sampler2DArray
	};

	class ShaderParamaterTypeStorageBase
	{
	public:
		virtual ShaderParameterType GetType() = 0;
		virtual void Bind() = 0;
	};

	class UBOShaderParameterTypeStorage : public ShaderParamaterTypeStorageBase
	{
	public:
		UBOShaderParameterTypeStorage(size_t size, uint32_t binding) { m_UBO = UniformBufferStorage::Create(size, binding); }

		virtual ShaderParameterType GetType() override { return ShaderParameterType::UBO; }
		virtual void Bind() override { m_UBO->UploadStorage(); }

		Ref<UniformBufferStorage> UBO() { return m_UBO; }
	private:
		Ref<UniformBufferStorage> m_UBO;
	};
}