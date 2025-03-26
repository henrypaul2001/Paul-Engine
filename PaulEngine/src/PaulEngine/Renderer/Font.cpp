#include "pepch.h"
#include "Font.h"

#include "PaulEngine/Project/Project.h"
#include "PaulEngine/Asset/AssetManager.h"
#include "PaulEngine/Asset/EditorAssetManager.h"
#include "PaulEngine/Renderer/MSDFData.h"

#include "Texture.h"

namespace PaulEngine
{
	Font::Font() : m_Data(new MSDFData()) {}

	Font::~Font()
	{
		delete m_Data;
	}

	Ref<Font> Font::GetDefault()
	{
		Ref<EditorAssetManager> assetManager = Project::GetActive()->GetEditorAssetManager();
		return AssetManager::GetAsset<Font>(assetManager->ImportAsset("../../assets/fonts/Open_Sans/static/OpenSans-Regular.ttf", true));
		
		//static Ref<Font> DefaultFont;
		//if (!DefaultFont) {
		//	DefaultFont = CreateRef<Font>("assets/fonts/Open_Sans/static/OpenSans-Regular.ttf");
		//}
		//return DefaultFont;
	}
}