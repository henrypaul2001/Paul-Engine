#include "pepch.h"
#include "Asset.h"

namespace PaulEngine
{
    static const std::unordered_map<std::string, AssetType> s_StringToType = {
        { "AssetType::None",                    AssetType::None },
        { "AssetType::Scene",                   AssetType::Scene },
        { "AssetType::Texture2D",               AssetType::Texture2D },
        { "AssetType::Font",                    AssetType::Font },
        { "AssetType::TextureAtlas2D",          AssetType::TextureAtlas2D },
        { "AssetType::Shader",                  AssetType::Shader },
        { "AssetType::Material",                AssetType::Material },
        { "AssetType::Texture2DArray",          AssetType::Texture2DArray },
        { "AssetType::TextureCubemap",          AssetType::TextureCubemap },
        { "AssetType::TextureCubemapArray",     AssetType::TextureCubemapArray },
        { "AssetType::Mesh",                    AssetType::Mesh },
        { "AssetType::Model",                   AssetType::Model },
        { "AssetType::Prefab",                  AssetType::Prefab },
        { "AssetType::EnvironmentMap",          AssetType::EnvironmentMap }
    };

    static const std::unordered_map<AssetType, std::string> s_TypeToString = {
        { AssetType::None                   , "AssetType::None"                 },
        { AssetType::Scene                  , "AssetType::Scene"                },
        { AssetType::Texture2D              , "AssetType::Texture2D"            },
        { AssetType::Font                   , "AssetType::Font"                 },
        { AssetType::TextureAtlas2D         , "AssetType::TextureAtlas2D"       },
        { AssetType::Shader                 , "AssetType::Shader"               },
        { AssetType::Material               , "AssetType::Material"             },
        { AssetType::Texture2DArray         , "AssetType::Texture2DArray"       },
        { AssetType::TextureCubemap         , "AssetType::TextureCubemap"       },
        { AssetType::TextureCubemapArray    , "AssetType::TextureCubemapArray"  },
        { AssetType::Mesh                   , "AssetType::Mesh"                 },
        { AssetType::Model                  , "AssetType::Model"                },
        { AssetType::Prefab                 , "AssetType::Prefab"               },
        { AssetType::EnvironmentMap         , "AssetType::EnvironmentMap"       }
    };

    std::string AssetTypeToString(AssetType type)
    {
        const auto it = s_TypeToString.find(type);
        if (it != s_TypeToString.end())
        {
            return it->second;
        }
        PE_CORE_WARN("Undefined asset type translation: {0}", (int)type);
        return "AssetType::!Unknown!";
    }

    AssetType AssetTypeFromString(const std::string& assetType)
    {
        const auto it = s_StringToType.find(assetType);
        if (it != s_StringToType.end())
        {
            return it->second;
        }
        PE_CORE_WARN("Undefined asset type translation: {0}", assetType);
        return AssetType::None;
    }
}