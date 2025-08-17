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
}
