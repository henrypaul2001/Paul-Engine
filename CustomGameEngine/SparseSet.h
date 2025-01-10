#pragma once
#include <vector>
#include <algorithm>
namespace Engine {
	template <class T>
	class SparseSet {
	public:
		SparseSet() {

		}
		~SparseSet() {

		}

		// Get functions
		T Get(const unsigned int index) const {
			assert(index < sparse.size());
			return dense[sparse[index]];
		}
		const T& GetRef(const unsigned int index) const {
			assert(index < sparse.size());
			return dense[sparse[index]];
		}
		T& GetRef(const unsigned int index) {
			assert(index < sparse.size());
			return dense[sparse[index]];
		}
		const T* GetPtr(const unsigned int index) const {
			assert(index < sparse.size());
			return &dense[sparse[index]];
		}
		T* GetPtr(const unsigned int index) {
			assert(index < sparse.size());
			return &dense[sparse[index]];
		}

		// Add functions
		
		// Will resize sparse vector if index is out of bounds. O(n) worst case
		bool Add(const unsigned int index, T value) {
			if (index >= sparse.size()) { sparse.resize(index + 1, -1); }

			return Set(index, value);
		}

		// Returns false if index is out of bounds of sparse set, true if value added to dense set with corresponding sparse set index pointer. O(1) complexity
		bool Set(const unsigned int index, T value) {
			if (index >= sparse.size()) { return false; }
			sparse[index] = dense.size();
			dense.push_back(value);
			denseToSparse.push_back(index);
			return true;
		}

		// Delete functions
		void Delete(const unsigned int index) {
			assert(index < sparse.size());
			int denseIndex = sparse[index];
			if (denseIndex == -1) { return; }

			std::swap(dense[denseIndex], dense[dense.size() - 1]);
			std::swap(denseToSparse[denseIndex], denseToSparse[denseToSparse.size() - 1]);

			// Update swapped sparse index
			sparse[denseToSparse[denseIndex]] = denseIndex;

			// Nullify deleted sparse index
			sparse[index] = -1;

			// Pop dense lists
			dense.pop_back();
			denseToSparse.pop_back();
		}

		const unsigned int DenseSize() const { return dense.size(); }
		const unsigned int SparseSize() const { return sparse.size(); }

	private:
		std::vector<T> dense;
		std::vector<unsigned int> denseToSparse;
		std::vector<int> sparse;
	};
}