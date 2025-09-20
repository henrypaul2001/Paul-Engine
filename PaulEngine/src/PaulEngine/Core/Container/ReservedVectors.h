#pragma once
#include <vector>
#include <optional>

namespace PaulEngine
{
	/// <summary>A vector of pre-reserved inner vectors.
	/// <para>
	/// An initial outer size can be provided and this will initialise n inner vectors, all of which will be called with reserve() and the templated InnerReserve value
	/// The outer vector will keep track of which inner vectors are in use, and unused inner vectors can be re-used without having to allocate any additional memory.
	/// </para></summary>
	template <typename T, size_t InnerReserve>
	class reserved_vectors
	{
	public:
		reserved_vectors(size_t outerSize = 5) : m_ActiveInnerVectors(0)
		{
			m_Vectors = std::vector<std::vector<T>>(outerSize);
			for (size_t i = 0; i < outerSize; i++)
			{
				std::vector<T>& innerVector = m_Vectors[i];
				innerVector.reserve(InnerReserve);
			}
		}

		inline const std::vector<std::vector<T>>& get_outer_vector() const { return m_Vectors; }
		inline std::span<std::vector<T>, std::dynamic_extent> get_inner_vectors()
		{
			std::span<std::vector<T>, std::dynamic_extent> span = std::span<std::vector<T>>(m_Vectors);
			return span.subspan(0, m_ActiveInnerVectors);
		}
		inline std::span<const std::vector<T>, std::dynamic_extent> get_inner_vectors() const
		{
			std::span<const std::vector<T>, std::dynamic_extent> span = std::span<const std::vector<T>>(m_Vectors);
			return span.subspan(0, m_ActiveInnerVectors);
		}

		inline const std::vector<T>& get_front() const { return get_inner_vector(0); }
		inline const std::vector<T>& get_back() const { return get_inner_vector(m_ActiveInnerVectors - 1); }
		inline const std::vector<T>& get_inner_vector(size_t index) const
		{
			PE_CORE_ASSERT(index < m_ActiveInnerVectors, "Index out of range");
			return m_Vectors[index];
		}

		inline std::vector<T>& get_front() { return get_inner_vector(0); }
		inline std::vector<T>& get_back() { return get_inner_vector(m_ActiveInnerVectors - 1); }
		inline std::vector<T>& get_inner_vector(size_t index)
		{
			PE_CORE_ASSERT(index < m_ActiveInnerVectors, "Index out of range");
			return m_Vectors[index];
		}

		std::vector<T>& create_new_inner_at_back()
		{
			m_ActiveInnerVectors++;
			if (m_ActiveInnerVectors > m_Vectors.size())
			{
				std::vector<T> newInnerVector = std::vector<T>();
				newInnerVector.reserve(InnerReserve);

				m_Vectors.push_back(newInnerVector);
			}
			std::vector<T>& back = m_Vectors[m_ActiveInnerVectors - 1];
			back.clear();
			return back;
		}

		std::vector<T>& create_new_inner_at_front()
		{
			m_ActiveInnerVectors++;
			if (m_ActiveInnerVectors > m_Vectors.size())
			{
				std::vector<T> newInnerVector = std::vector<T>();
				newInnerVector.reserve(InnerReserve);
				m_Vectors.insert(m_Vectors.begin(), newInnerVector);
			}
			else
			{
				// Re-use dead vector from back
				std::vector<T> backCopy = m_Vectors[m_Vectors.size() - 1];
				backCopy.clear();
				m_Vectors.pop_back();
				m_Vectors.insert(m_Vectors.begin(), backCopy);
			}
			return m_Vectors[0];
		}

		void pop_back()
		{
			if (m_ActiveInnerVectors > 0)
			{
				m_ActiveInnerVectors--;
			}
		}

		void clear()
		{
			m_ActiveInnerVectors = 0;
		}

		// Will call reserve() on any newly created inner vectors
		void resize_outer(size_t newSize)
		{
			size_t oldSize = m_Vectors.size();
			m_Vectors.resize(newSize);
			
			if (m_ActiveInnerVectors > newSize)
			{
				m_ActiveInnerVectors = newSize;
			}
			else
			{
				for (size_t i = oldSize; i < newSize; i++)
				{
					m_Vectors[i].reserve(InnerReserve);
				}
			}
		}

		inline size_t size() const { return m_ActiveInnerVectors; }

		inline std::vector<T>& operator[](size_t index)
		{
			PE_CORE_ASSERT(index < m_ActiveInnerVectors, "Index out of range");
			return m_Vectors[index];
		}
		inline const std::vector<T>& operator[](size_t index) const
		{
			PE_CORE_ASSERT(index < m_ActiveInnerVectors, "Index out of range");
			return m_Vectors[index];
		}

	private:
		std::vector<std::vector<T>> m_Vectors;

		size_t m_ActiveInnerVectors;
	};
}