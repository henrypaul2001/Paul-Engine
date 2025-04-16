#pragma once
#include "PaulEngine/Core/Core.h"
#include "UniformBufferStorage.h"
#include "PaulEngine/Asset/Asset.h"

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
		virtual ~ShaderParamaterTypeStorageBase() {}
		virtual ShaderParameterType GetType() = 0;
		virtual void Bind() = 0;
	};

	class UBOShaderParameterTypeStorage : public ShaderParamaterTypeStorageBase
	{
	public:
		UBOShaderParameterTypeStorage(size_t size, uint32_t binding) { m_UBO = UniformBufferStorage::Create(size, binding); }
		~UBOShaderParameterTypeStorage() {}

		virtual ShaderParameterType GetType() override { return ShaderParameterType::UBO; }
		virtual void Bind() override { m_UBO->Bind(); m_UBO->UploadStorage(); }

		Ref<UniformBufferStorage> UBO() { return m_UBO; }
	private:
		Ref<UniformBufferStorage> m_UBO;
	};

	class Sampler2DShaderParameterTypeStorage : public ShaderParamaterTypeStorageBase
	{
	public:
		Sampler2DShaderParameterTypeStorage(AssetHandle textureHandle, uint32_t binding) : m_TextureHandle(textureHandle), m_Binding(binding) {}
		~Sampler2DShaderParameterTypeStorage() {}

		virtual ShaderParameterType GetType() override { return ShaderParameterType::Sampler2D; }
		virtual void Bind() override;

		AssetHandle GetTextureHandle() const { return m_TextureHandle; }
		uint32_t GetBinding() const { return m_Binding; }

	private:
		friend class EditorLayer;
		AssetHandle m_TextureHandle;
		uint32_t m_Binding;
	};

	class Sampler2DArrayShaderParameterTypeStorage : public ShaderParamaterTypeStorageBase
	{
	public:
		Sampler2DArrayShaderParameterTypeStorage(AssetHandle textureArrayHandle, uint32_t binding) : m_TextureArrayHandle(textureArrayHandle), m_Binding(binding) {}
		~Sampler2DArrayShaderParameterTypeStorage() {}

		virtual ShaderParameterType GetType() override { return ShaderParameterType::Sampler2DArray; }
		virtual void Bind() override;

		AssetHandle GetTextureArrayHandle() const { return m_TextureArrayHandle; }
		uint32_t GetBinding() const { return m_Binding; }

	private:
		friend class EditorLayer;
		AssetHandle m_TextureArrayHandle;
		uint32_t m_Binding;
	};

	struct ShaderParameterTypeSpecifcationBase
	{
		uint32_t Binding = 0;
		std::string Name = "";
		virtual ShaderParameterType Type() const = 0;
	};

	struct UBOShaderParameterTypeSpecification : public ShaderParameterTypeSpecifcationBase
	{
		struct LayoutElement {
			ShaderDataType Type;
			std::string Name;
		};
		std::vector<LayoutElement> BufferLayout;

		 virtual ShaderParameterType Type() const override { return ShaderParameterType::UBO; }
	};

	struct Sampler2DShaderParameterTypeSpecification : public ShaderParameterTypeSpecifcationBase
	{
		virtual ShaderParameterType Type() const override { return ShaderParameterType::Sampler2D; }
	};
	
	struct Sampler2DArrayShaderParameterTypeSpecification : public ShaderParameterTypeSpecifcationBase
	{
		virtual ShaderParameterType Type() const override { return ShaderParameterType::Sampler2DArray; }
	};
}