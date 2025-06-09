#pragma once
#include "PaulEngine/Events/Event.h"
#include "PaulEngine/Asset/Asset.h"

namespace PaulEngine
{
	class AssetImportedEvent : public Event
	{
	public:
		AssetImportedEvent(AssetHandle importedAsset, bool newlyRegistered) : m_ImportedAsset(importedAsset), m_IsNewlyRegistered(newlyRegistered) {}

		inline AssetHandle GetAssetHandle() const { return m_ImportedAsset; }
		inline bool IsNewlyRegistered() const { return m_IsNewlyRegistered; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "AssetImportedEvent: " << m_ImportedAsset << ". IsNewlyRegistered: " << m_IsNewlyRegistered;
			return ss.str();
		}

		EVENT_CLASS_TYPE(AssetImported)
		EVENT_CLASS_CATEGORY(EventCategoryAsset)

	private:
		AssetHandle m_ImportedAsset;
		bool m_IsNewlyRegistered;
	};
}