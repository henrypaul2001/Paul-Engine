#include "pepch.h"
#include "Asset.h"

namespace PaulEngine
{
    std::string AssetTypeToString(AssetType type)
    {
        switch (type)
        {
            case AssetType::None: return "AssetType::None";
            case AssetType::Scene: return "AssetType::Scene";
            case AssetType::Texture2D: return "AssetType::Texture2D";
            case AssetType::Font: return "AssetType::Font";
            case AssetType::TextureAtlas2D: return "AssetType::TextureAtlas2D";
            case AssetType::Shader: return "AssetType::Shader";
        }
        return "AssetType::!Unknown!";
    }

    AssetType AssetTypeFromString(const std::string& assetType)
    {
        if (assetType == "AssetType::None") { return AssetType::None; }
        if (assetType == "AssetType::Scene") { return AssetType::Scene; }
        if (assetType == "AssetType::Texture2D") { return AssetType::Texture2D; }
        if (assetType == "AssetType::Font") { return AssetType::Font; }
        if (assetType == "AssetType::TextureAtlas2D") { return AssetType::TextureAtlas2D; }
        if (assetType == "AssetType::Shader") { return AssetType::Shader; }
        return AssetType::None;
    }
}