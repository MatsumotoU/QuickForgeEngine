#pragma once
#include <vector>
#include <cstdint>
template<typename T>
class SparseSets {
public:
	SparseSet() = default;
	void Insert(uint32_t key, const T& value) {
		if (key >= sparse_.size()) {
			sparse_.resize(key + 1, -1);
		}
		if (sparse_[key] == -1) {
			dense_.push_back(value);
			sparse_[key] = static_cast<int32_t>(dense_.size() - 1);
		} else {
			dense_[sparse_[key]] = value;
		}
	}
	void Remove(uint32_t key) {
		if (key < sparse_.size() && sparse_[key] != -1) {
			int32_t index = sparse_[key];
			dense_[index] = dense_.back();
			dense_.pop_back();
			sparse_[key] = -1;
			for (uint32_t i = 0; i < sparse_.size(); ++i) {
				if (sparse_[i] == static_cast<int32_t>(dense_.size())) {
					sparse_[i] = index;
					break;
				}
			}
		}
	}
	T* Get(uint32_t key) {
		if (key < sparse_.size() && sparse_[key] != -1) {
			return &dense_[sparse_[key]];
		}
		return nullptr;
	}
	bool Contains(uint32_t key) const {
		return key < sparse_.size() && sparse_[key] != -1;
	}
	const std::vector<T>& GetDenseArray() const {
		return dense_;
	}

	// 標準ライブラリ風インターフェース
	auto begin() { return dense_.begin(); }
	auto end() { return dense_.end(); }
	auto begin() const { return dense_.begin(); }
	auto end() const { return dense_.end(); }
	size_t size() const { return dense_.size(); }
	bool empty() const { return dense_.empty(); }
	void erase(uint32_t key) { Remove(key); }
	T* find(uint32_t key) { return Get(key); }
	T& operator[](uint32_t key) {
		if (!Contains(key)) {
			Insert(key, T{});
		}
		return dense_[sparse_[key]];
	}
private:
	std::vector<int32_t> sparse_;
	std::vector<T> dense_;
};