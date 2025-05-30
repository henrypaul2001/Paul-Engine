#include "pepch.h"
#include "Font.h"

#include "PaulEngine/Project/Project.h"
#include "PaulEngine/Asset/AssetManager.h"
#include "PaulEngine/Asset/EditorAssetManager.h"
#include "PaulEngine/Renderer/Asset/MSDFData.h"

#include "Texture.h"

namespace PaulEngine
{
	AssetHandle Font::s_DefaultFont = 0;
	Font::Font() : m_Data(new MSDFData()) {}

	Font::~Font()
	{
		delete m_Data;
	}
}