#pragma once
#include "engine/include/core/Memory/SafeVector.h"
#include <cstdint>
namespace QFE {
	/// @brief スパースセットコンテナ
	template<typename T>
	class SparseSet {
	public:
		SparseSet() = default;
		/// @brief キーと値のペアを追加する。キーは自動的に割り当てられる。
		uint32_t push_back(const T& value) {
			// 未使用のキー（-1の場所）を探す
			uint32_t key = 0;
			while (key < sparse_.size() && sparse_[key] != -1) {
				key++;
			}
			
			// key に到達するまで push_back でサイズを拡大（空きは -1 で埋める）
			while (key >= sparse_.size()) {
				sparse_.push_back(-1);
			}

			dense_.push_back(value);
			sparse_[key] = static_cast<int32_t>(dense_.size() - 1);
			return key;
		}

		/// @brief キーと値のペアを追加または更新する。キーが存在しない場合は新しいペアを追加し、存在する場合は値を更新する。
		void Insert(uint32_t key, const T& value) {
			// 指定された key にアクセスできるようになるまでサイズを広げる
			while (key >= sparse_.size()) {
				sparse_.push_back(-1);
			}

			if (sparse_[key] == -1) {
				dense_.push_back(value);
				sparse_[key] = static_cast<int32_t>(dense_.size() - 1);
			} else {
				dense_[sparse_[key]] = value;
			}
		}
		/// @brief キーに対応する値を削除する。キーが存在しない場合は何もしない。
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
		/// @brief キーに対応する値へのポインタを取得する。キーが存在しない場合はnullptrを返す。
		T* Get(uint32_t key) {
			if (key < sparse_.size() && sparse_[key] != -1) {
				return &dense_[sparse_[key]];
			}
			return nullptr;
		}
		/// @brief キーに対応する要素が存在するかどうかを確認する。
		bool Contains(uint32_t key) const {
			return key < sparse_.size() && sparse_[key] != -1;
		}
		/// @brief キーの一覧を取得する。
		std::vector<uint32_t> Keys() const {
			std::vector<uint32_t> keys;
			for (uint32_t i = 0; i < sparse_.size(); ++i) {
				if (sparse_[i] != -1) {
					keys.push_back(i);
				}
			}
			return keys;
		}

		// 標準ライブラリ風インターフェース
		void clear() {
			sparse_.clear();
			dense_.clear();
		}
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
		T& at(uint32_t key) {
			return dense_.at(sparse_.at(key));
		}
	private:
		SafeVector<int32_t> sparse_;
		SafeVector<T> dense_;
	};
} // namespace QFE
