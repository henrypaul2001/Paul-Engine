#pragma once
#include <vector>
#include <unordered_map>
#include <optional>

namespace PaulEngine
{
	// A utility class to manage an std::vector that only allows unique elements backed by an unordered_map but keeps insertion order
	template <typename Key, typename Value>
	class mapped_vector
	{
	public:
		mapped_vector() : m_Values(), m_Map() {}
		mapped_vector(size_t initialCapacity) : m_Values(), m_Map() { reserve(initialCapacity); }

		inline const std::unordered_map<Key, size_t>& map() const { return m_Map; }
		inline const std::vector<Value>& vector() const { return m_Values; }
		
		inline size_t size() const { return m_Values.size(); }
		
		inline std::unordered_map<Key, size_t>::const_iterator find(const Key& key) const { return m_Map.find(key); }
		inline bool contains(const Key& key) const { return m_Map.contains(key); }

		inline void clear() { m_Map.clear(); m_Values.clear(); }
		inline void reserve(size_t newCapacity) { m_Values.reserve(newCapacity); }
		inline void shrink_to_fit() { m_Values.shrink_to_fit(); }

		inline std::optional<size_t> get_index(const Key& key) const
		{
			auto it = find(key);
			if (it != m_Map.end())
			{
				return it->second;
			}
			else { return std::nullopt; }
		}

		std::optional<size_t> try_push_back(const Key& key, const Value& value)
		{
			if (!contains(key))
			{
				size_t index = m_Values.size();
				m_Values.push_back(value);
				m_Map[key] = index;
				return index;
			}
			else { return std::nullopt; }
		}

		template <typename... Args>
		std::optional<size_t> try_emplace_back(const Key& key, Args&&... args)
		{
			if (!contains(key))
			{
				size_t index = m_Values.size();
				m_Values.emplace_back(std::forward<Args>(args)...);
				m_Map[key] = index;
				return index;
			}
			else { return std::nullopt; }
		}

		// returns <index, true> if key already exists
		std::pair<size_t, bool> get_index_or_push_back(const Key& key, const Value& value)
		{
			std::optional<size_t> index = get_index(key);
			if (index)
			{
				return std::make_pair(index.value(), true);
			}
			else
			{
				size_t index = m_Values.size();
				m_Values.push_back(value);
				m_Map[key] = index;
				return std::make_pair(index, false);
			}
		}

		// returns <index, true> if key already exists
		template <typename... Args>
		std::pair<size_t, bool> get_index_or_emplace_back(const Key& key, Args&&... args)
		{
			std::optional<size_t> index = get_index(key);
			if (index)
			{
				return std::make_pair(index.value(), true);
			}
			else
			{
				size_t index = m_Values.size();
				m_Values.emplace_back(std::forward<Args>(args)...);
				m_Map[key] = index;
				return std::make_pair(index, false);
			}
		}

		Value& operator [](size_t index)
		{
			return m_Values[index];
		}
		const Value& operator[](size_t index) const
		{
			return m_Values[index];
		}

		inline std::optional<size_t> operator[](const Key& key) const { get_index(key); }

	private:
		std::unordered_map<Key, size_t> m_Map;
		std::vector<Value> m_Values;
	};
}