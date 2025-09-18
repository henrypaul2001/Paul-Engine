#include "pepch.h"
#include "ShaderParameterType.h"

#include "PaulEngine/Asset/AssetManager.h"
#include "PaulEngine/Renderer/Asset/Texture.h"

#include "PaulEngine/Renderer/Renderer2D.h"

namespace PaulEngine
{
	void Sampler2DShaderParameterTypeStorage::Bind()
	{
		Ref<Texture2D> texture = AssetManager::GetAsset<Texture2D>(TextureHandle);
		if (texture)
		{
			texture->Bind(m_Binding);
		}
		else
		{
			Renderer2D::GetWhiteTexture()->Bind(m_Binding);
		}
	}

	void Sampler2DArrayShaderParameterTypeStorage::Bind()
	{
		Ref<Texture2DArray> textureArray = AssetManager::GetAsset<Texture2DArray>(TextureArrayHandle);
		if (textureArray)
		{
			textureArray->Bind(m_Binding);
		}
		else
		{
			// TODO: Bind empty texture array
		}
	}

	void SamplerCubeShaderParameterTypeStorage::Bind()
	{
		Ref<TextureCubemap> texture = AssetManager::GetAsset<TextureCubemap>(TextureHandle);
		if (texture)
		{
			texture->Bind(m_Binding);
		}
		else
		{
			// TODO: Bind empty cubemap
		}
	}

	void SamplerCubeArrayShaderParameterTypeStorage::Bind()
	{
		Ref<TextureCubemapArray> textureArray = AssetManager::GetAsset<TextureCubemapArray>(TextureArrayHandle);
		if (textureArray)
		{
			textureArray->Bind(m_Binding);
		}
		else
		{
			// TODO: Bind empty cubemap array
		}
	}

	void StorageBufferEntryShaderParameterTypeStorage::Bind()
	{
		PatchAndSetData();
	}

	void StorageBufferEntryShaderParameterTypeStorage::BindlessUpload(uint32_t materialIndex)
	{
		size_t offset = m_LocalBuffer.Size() * materialIndex;
		size_t end = offset + m_LocalBuffer.Size();

		if (m_Capacity > -1)
		{
			// Validate buffer size
			if (end > m_Capacity)
			{
				PE_CORE_ASSERT(false, "Buffer overrun");
				return;
			}
		}

		PatchAndSetData(offset);
	}

	void StorageBufferEntryShaderParameterTypeStorage::PatchAndSetData(size_t offset)
	{
		PE_PROFILE_FUNCTION();
		const uint8_t* data = m_LocalBuffer.Data();

		// TODO: Look into building a GPU indirection table to pass AssetHandle types directly to shaders and allow the shader to map and create the appropriate sampler (would require a rework on reflecting sampler types as they would just be represented as uint64s in GLSL)
		// Patch local buffer texture handles
		const std::vector<std::string>& textureMembers = m_LocalBuffer.GetTextureMemberNames();
		std::vector<AssetHandle> assetHandles = std::vector<AssetHandle>(textureMembers.size());

		for (size_t i = 0; i < textureMembers.size(); i++)
		{
			const std::string& name = textureMembers[i];

			AssetHandle textureHandle = 0;
			m_LocalBuffer.ReadLocalMemberAs(name, textureHandle);
			assetHandles[i] = textureHandle;

			uint64_t deviceHandle = DeviceHandleTracker::AssetHandleToDeviceHandle(textureHandle);
			if (deviceHandle == 0)
			{
				deviceHandle = Renderer2D::GetWhiteTexture()->GetDeviceTextureHandle();
			}

			m_LocalBuffer.SetLocalMember(name, deviceHandle);
		}

		m_StorageBufferContext->SetData({ (const void*)data, m_LocalBuffer.Size(), offset }, true);

		// Unpatch local buffer texture handles
		for (size_t i = 0; i < textureMembers.size(); i++)
		{
			const std::string& name = textureMembers[i];
			m_LocalBuffer.SetLocalMember(name, assetHandles[i]);
		}
	}
}
