#include "pepch.h"
#include "ShaderParameterType.h"

#include "PaulEngine/Asset/AssetManager.h"
#include "PaulEngine/Renderer/Texture.h"

#include "PaulEngine/Renderer/Renderer2D.h"

namespace PaulEngine
{
	void Sampler2DShaderParameterTypeStorage::Bind()
	{
		Ref<Texture2D> texture = AssetManager::GetAsset<Texture2D>(m_TextureHandle);
		if (texture)
		{
			texture->Bind(m_Binding);
		}
		else
		{
			Renderer2D::GetWhiteTexture()->Bind(m_Binding);
		}
	}
}
