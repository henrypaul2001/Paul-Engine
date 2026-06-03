#pragma once
#include "PaulEngine/Events/Event.h"
#include "PaulEngine/Asset/Asset.h"

namespace PaulEngine {
	class ContentBrowserAssetSelected : public Event
	{
	public:
		ContentBrowserAssetSelected(AssetHandle selectedAsset) : m_SelectedAsset(selectedAsset) {}

		AssetHandle GetHandle() const { return m_SelectedAsset; }

		std::string ToString() const override {
			std::stringstream ss;
			ss << "ContentBrowserAssetSelected: Handle = " << (UUID)m_SelectedAsset;
			return ss.str();
		}

		EVENT_CLASS_TYPE(AssetSelected)
		EVENT_CLASS_CATEGORY(EventCategoryEditor | EventCategoryAsset)

	private:
		AssetHandle m_SelectedAsset;
	};
}