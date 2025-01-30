#pragma once
#include <vector>
#include <algorithm>
#include <cassert>
namespace Engine {

	class ISparseSet {
	public:
		virtual ~ISparseSet() = default;
		virtual bool Delete(const unsigned int index) = 0;
		virtual const size_t DenseSize() const = 0;
		virtual const size_t SparseSize() const = 0;
		virtual bool CloneElement(const unsigned int sparseIDOrigin, const unsigned int sparseIDDestination) = 0;
		virtual const std::vector<unsigned int>& GetDenseToSparse() const = 0;
		virtual const bool ValidateIndex(const unsigned int sparseIndex) const = 0;
	};

	template <class T>
	class SparseSet : public ISparseSet {
	public:
		SparseSet(const unsigned int size = 0u, const unsigned int denseReserve = 3u) {
			sparse = std::vector<int>(size, -1);
			dense.reserve(denseReserve);
		}

		// Get functions
		T Get(const unsigned int index) const {
			assert(index < sparse.size());
			const int denseIndex = sparse[index];
			assert(denseIndex != -1);
			return dense[denseIndex];
		}
		const T& GetRef(const unsigned int index) const {
			assert(index < sparse.size());
			const int denseIndex = sparse[index];
			assert(denseIndex != -1);
			return dense[denseIndex];
		}
		T& GetRef(const unsigned int index) {
			assert(index < sparse.size());
			const int denseIndex = sparse[index];
			assert(denseIndex != -1);
			return dense[denseIndex];
		}
		const T* GetPtr(const unsigned int index) const {
			assert(index < sparse.size());
			const int denseIndex = sparse[index];
			if (denseIndex != -1) { return &dense[denseIndex]; }
			else { return nullptr; }
		}
		T* GetPtr(const unsigned int index) {
			assert(index < sparse.size());
			const int denseIndex = sparse[index];
			if (denseIndex != -1) { return &dense[denseIndex]; }
			else { return nullptr; }
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

		// Delete entry at index. Returns false if sparse index doesn't point to a dense entry
		bool Delete(const unsigned int index) override {
			if (!ValidateIndex(index)) { return false; }

			const int denseIndex = sparse[index];

			std::swap(dense[denseIndex], dense[dense.size() - 1]);
			std::swap(denseToSparse[denseIndex], denseToSparse[denseToSparse.size() - 1]);

			// Update swapped sparse index
			sparse[denseToSparse[denseIndex]] = denseIndex;

			// Nullify deleted sparse index
			sparse[index] = -1;

			// Pop dense lists
			memset(&dense[dense.size() - 1], NULL, sizeof(dense[dense.size() - 1]));
			dense.pop_back();
			denseToSparse.pop_back();

			return true;
		}

		bool CloneElement(const unsigned int sparseIDOrigin, const unsigned int sparseIDDestination) override {
			return Add(sparseIDDestination, T(GetRef(sparseIDOrigin)));
		}

		const size_t DenseSize() const override { return dense.size(); }
		const size_t SparseSize() const override { return sparse.size(); }

		const std::vector<T>& Dense() const { return dense; }
		const std::vector<unsigned int>& GetDenseToSparse() const override { return denseToSparse; }

		const unsigned int GetSparseIndexFromDense(const unsigned int index) const { return denseToSparse[index]; }

		const bool ValidateIndex(const unsigned int sparseIndex) const override { 
			return (sparseIndex < sparse.size() && sparse[sparseIndex] != -1); 
		}

	private:
		std::vector<T> dense;
		std::vector<unsigned int> denseToSparse;
		std::vector<int> sparse;
	};
}