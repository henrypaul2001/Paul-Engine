#pragma once
#include <vector>
#include <algorithm>
namespace Engine {

	class ISparseSet {
	public:
		virtual ~ISparseSet() = default;
		virtual void Delete(const unsigned int index) = 0;
		virtual const size_t DenseSize() const = 0;
		virtual const size_t SparseSize() const = 0;
	};

	template <class T>
	class SparseSet : public ISparseSet {
	public:
		SparseSet(const unsigned int size = 10u, const unsigned int denseReserve = 3u) {
			sparse = std::vector<int>(10, -1);
			dense.reserve(denseReserve);
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
		void Delete(const unsigned int index) override {
			assert(index < sparse.size());
			const int denseIndex = sparse[index];
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

		const size_t DenseSize() const override { return dense.size(); }
		const size_t SparseSize() const override { return sparse.size(); }

		const std::vector<T>& Dense() const { return dense; }

	private:
		std::vector<T> dense;
		std::vector<unsigned int> denseToSparse;
		std::vector<int> sparse;
	};
}