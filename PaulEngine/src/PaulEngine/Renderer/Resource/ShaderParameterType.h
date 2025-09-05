#pragma once
#include "PaulEngine/Core/Core.h"
#include "ShaderStorageBuffer.h"
#include "UniformBufferStorage.h"
#include "PaulEngine/Asset/Asset.h"

namespace PaulEngine
{
	enum class ShaderParameterType
	{
		None = 0,
		UBO,
		Sampler2D,
		Sampler2DArray,
		SamplerCube,
		SamplerCubeArray,
		SSBO
	};

	class ShaderParamaterTypeStorageBase
	{
	public:
		virtual ~ShaderParamaterTypeStorageBase() {}
		virtual ShaderParameterType GetType() const = 0;
		virtual void Bind() = 0;
	};

	class UBOShaderParameterTypeStorage : public ShaderParamaterTypeStorageBase
	{
	public:
		UBOShaderParameterTypeStorage(std::vector<BufferElement> layout, uint32_t binding) { m_UBO = UniformBufferStorage::Create(layout, binding); }
		~UBOShaderParameterTypeStorage() {}

		virtual ShaderParameterType GetType() const override { return ShaderParameterType::UBO; }
		virtual void Bind() override { m_UBO->Bind(); m_UBO->UploadStorage(); }

		Ref<UniformBufferStorage> UBO() { return m_UBO; }

	private:
		friend class EditorLayer;
		friend class CreateMaterialWindow;

		Ref<UniformBufferStorage> m_UBO;
	};

	// Used as a buffer instance to buffer into a specific index of a larger storage buffer array
	class StorageBufferEntryShaderParameterTypeStorage : public ShaderParamaterTypeStorageBase
	{
	public:
		StorageBufferEntryShaderParameterTypeStorage(std::vector<BufferElement> layout, Ref<ShaderStorageBuffer> storageBufferContext, int32_t capacity) : m_LocalBuffer(layout), m_StorageBufferContext(storageBufferContext), m_Capacity(capacity) {}
		~StorageBufferEntryShaderParameterTypeStorage() {}

		virtual ShaderParameterType GetType() const override { return ShaderParameterType::SSBO; }

		// buffer into the base index of the storage buffer as if this instance is the entire storage buffer
		virtual void Bind() override;
		void BindlessUpload(uint32_t materialIndex);

		inline const LocalShaderBuffer& GetLocalBuffer() const { return m_LocalBuffer; }
		inline LocalShaderBuffer& GetLocalBuffer() { return m_LocalBuffer; }

		// -1 = dynamic capacity
		inline const int32_t Capacity() const { return m_Capacity; }

	private:
		friend class EditorLayer;
		friend class CreateMaterialWindow;

		int32_t m_Capacity; // -1 = dynamic capacity
		Ref<ShaderStorageBuffer> m_StorageBufferContext;
		LocalShaderBuffer m_LocalBuffer;
	};

	class Sampler2DShaderParameterTypeStorage : public ShaderParamaterTypeStorageBase
	{
	public:
		Sampler2DShaderParameterTypeStorage(AssetHandle textureHandle, uint32_t binding) : TextureHandle(textureHandle), m_Binding(binding) {}
		~Sampler2DShaderParameterTypeStorage() {}

		virtual ShaderParameterType GetType() const override { return ShaderParameterType::Sampler2D; }
		virtual void Bind() override;

		uint32_t GetBinding() const { return m_Binding; }

		AssetHandle TextureHandle;
	private:
		friend class EditorLayer;
		friend class CreateMaterialWindow;
		uint32_t m_Binding;
	};

	class Sampler2DArrayShaderParameterTypeStorage : public ShaderParamaterTypeStorageBase
	{
	public:
		Sampler2DArrayShaderParameterTypeStorage(AssetHandle textureArrayHandle, uint32_t binding) : TextureArrayHandle(textureArrayHandle), m_Binding(binding) {}
		~Sampler2DArrayShaderParameterTypeStorage() {}

		virtual ShaderParameterType GetType() const override { return ShaderParameterType::Sampler2DArray; }
		virtual void Bind() override;

		uint32_t GetBinding() const { return m_Binding; }

		AssetHandle TextureArrayHandle;
	private:
		friend class EditorLayer;
		friend class CreateMaterialWindow;
		uint32_t m_Binding;
	};

	class SamplerCubeShaderParameterTypeStorage : public ShaderParamaterTypeStorageBase
	{
	public:
		SamplerCubeShaderParameterTypeStorage(AssetHandle textureHandle, uint32_t binding) : TextureHandle(textureHandle), m_Binding(binding) {}
		~SamplerCubeShaderParameterTypeStorage() {}

		virtual ShaderParameterType GetType() const override { return ShaderParameterType::SamplerCube; }
		virtual void Bind() override;

		uint32_t GetBinding() const { return m_Binding; }

		AssetHandle TextureHandle;
	private:
		friend class EditorLayer;
		friend class CreateMaterialWindow;
		uint32_t m_Binding;
	};

	class SamplerCubeArrayShaderParameterTypeStorage : public ShaderParamaterTypeStorageBase
	{
	public:
		SamplerCubeArrayShaderParameterTypeStorage(AssetHandle textureArrayHandle, uint32_t binding) : TextureArrayHandle(textureArrayHandle), m_Binding(binding) {}
		~SamplerCubeArrayShaderParameterTypeStorage() {}

		virtual ShaderParameterType GetType() const override { return ShaderParameterType::SamplerCubeArray; }
		virtual void Bind() override;

		uint32_t GetBinding() const { return m_Binding; }

		AssetHandle TextureArrayHandle;
	private:
		friend class EditorLayer;
		friend class CreateMaterialWindow;
		uint32_t m_Binding;
	};

	struct ShaderParameterTypeSpecificationBase
	{
		std::string Name = "";
		virtual ShaderParameterType Type() const = 0;
	};

	struct UBOShaderParameterTypeSpecification : public ShaderParameterTypeSpecificationBase
	{
		uint32_t Size = 0;
		uint32_t Binding = 0;
		std::vector<BufferElement> BufferLayout;

		 virtual ShaderParameterType Type() const override { return ShaderParameterType::UBO; }
	};

	struct Sampler2DShaderParameterTypeSpecification : public ShaderParameterTypeSpecificationBase
	{
		uint32_t Binding = 0;
		virtual ShaderParameterType Type() const override { return ShaderParameterType::Sampler2D; }
	};
	
	struct Sampler2DArrayShaderParameterTypeSpecification : public ShaderParameterTypeSpecificationBase
	{
		uint32_t Binding = 0;
		virtual ShaderParameterType Type() const override { return ShaderParameterType::Sampler2DArray; }
	};

	struct SamplerCubeShaderParameterTypeSpecification : public ShaderParameterTypeSpecificationBase
	{
		uint32_t Binding = 0;
		virtual ShaderParameterType Type() const override { return ShaderParameterType::SamplerCube; }
	};

	struct SamplerCubeArrayShaderParameterTypeSpecification : public ShaderParameterTypeSpecificationBase
	{
		uint32_t Binding = 0;
		virtual ShaderParameterType Type() const override { return ShaderParameterType::SamplerCubeArray; }
	};

	struct StorageBufferShaderParameterTypeSpecification : public ShaderParameterTypeSpecificationBase
	{
		uint32_t Size = 0;
		uint32_t Binding = 0;
		std::vector<BufferElement> BufferLayout;
		int32_t DynamicArrayStart = std::numeric_limits<int32_t>::max(); // -1 means SSBO is fixed size with no runtime sized array

		virtual ShaderParameterType Type() const override { return ShaderParameterType::SSBO; }
	};
}