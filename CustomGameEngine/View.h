#pragma once
#include <functional>
#include "SparseSet.h"
#include <array>

namespace Engine {
	template <class... Types>
	struct TypeList {
		using type_tuple = std::tuple<Types...>;

		template <std::size_t index>
		using get = std::tuple_element_t<index, type_tuple>;

		static constexpr std::size_t size = sizeof...(Types);
	};

	template <typename... Components>
	class View {
	public:
		using ForEachFunc = std::function<void(const unsigned int, Components&...)>;

		View(std::array<ISparseSet*, sizeof...(Components)> pools) : viewPools{ pools } {
			assert(componentTypes::size == viewPools.size());

			// Find smallest component pool for basis of for each loop
			smallestPool = viewPools[0];
			for (int i = 1; i < viewPools.size(); i++) {
				if (viewPools[i]->DenseSize() < smallestPool->DenseSize()) { smallestPool = viewPools[i]; }
			}
		}

		// Execute function on each element in view
		// [](const unsigned int sparseID, Components& c1, Components& c2, ...)
		void ForEach(ForEachFunc func) {
			auto indices = std::make_index_sequence<sizeof...(Components)>{};

			// Iterate smallest pool and execute function only if all other pools share this id
			for (const unsigned int id : smallestPool->GetDenseToSparse()) {
				if (AllContains(id)) {
					std::apply(func, std::tuple_cat(std::make_tuple(id), MakeComponentTuple(id, indices)));
				}
			}
		}

		struct Pack {
			unsigned int id;
			std::tuple<Components&...> components;
		};

		// Return a collection of packs in the view. Where each pack represents a sparse index and it's associated dense components. Useful for indexed iteration
		std::vector<Pack> GetPacked() {
			auto indices = std::make_index_sequence<sizeof... (Components)>{};
			std::vector<Pack> result;

			for (const unsigned int id : smallestPool->GetDenseToSparse()) {
				if (AllContains(id)) {
					result.push_back({ id, MakeComponentTuple(id, indices) });
				}
			}

			return result;
		}
	private:
		using componentTypes = TypeList<Components...>;
		
		// Get pool by index and downcast to SparseSet<type> using compile time indices
		template <std::size_t index>
		auto GetPoolAt() {
			using componentType = typename componentTypes::template get<index>;
			return static_cast<SparseSet<componentType>*>(viewPools[index]);
		}

		template <std::size_t... indices>
		auto MakeComponentTuple(const unsigned int id, std::index_sequence<indices...>) {
			return std::make_tuple((std::ref(GetPoolAt<indices>()->GetRef(id)))...);
		}

		// Returns true if all pools contain given id
		bool AllContains(const unsigned int id) {
			for (int i = 0; i < viewPools.size(); i++) {
				if (!viewPools[i]->ValidateIndex(id)) { return false; }
			}
			return true;
		}

		std::array<ISparseSet*, sizeof...(Components)> viewPools;
		ISparseSet* smallestPool;
	};
}